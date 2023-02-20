// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2023, KNS Group LLC (YADRO)

#include <time.h>

#include <host_config_cmd.hpp>
#include <ipmid/utils.hpp>
#include <oem_cmd.hpp>
#include <phosphor-logging/log.hpp>

#include <filesystem>
#include <fstream>

using namespace phosphor::logging;

namespace ipmi
{
namespace yadro
{
namespace host_config
{

/** @brief Register IPMI OEM command handler. */
void registerHostConfigHandlers() __attribute__((constructor));

/**
 * @brief Convert PCIe bifurcation mode to UEFI representation
 */
uint8_t convertBifurcation(const PCIe::BifurcationMode bifurcation,
                           const uint8_t fallback)
{
    switch (bifurcation)
    {
        case PCIe::BifurcationMode::x4x4x4x4:
            return pcieBifurcateX4X4X4X4;
        case PCIe::BifurcationMode::x4x4x8:
            return pcieBifurcateX4X4XXX8;
        case PCIe::BifurcationMode::x8x4x4:
            return pcieBifurcateXXX8X4X4;
        case PCIe::BifurcationMode::x8x8:
            return pcieBifurcateXXX8XXX8;
        case PCIe::BifurcationMode::x16:
            return pcieBifurcateXXXXXX16;

        case PCIe::BifurcationMode::lo_x8:
        {
            switch (fallback)
            {
                case pcieBifurcateX4X4X4X4:
                case pcieBifurcateX4X4XXX8:
                    return pcieBifurcateX4X4XXX8;
                default:
                    return pcieBifurcateXXX8XXX8;
            }
            break;
        }
        case PCIe::BifurcationMode::lo_x4x4:
        {
            switch (fallback)
            {
                case pcieBifurcateX4X4X4X4:
                case pcieBifurcateX4X4XXX8:
                    return pcieBifurcateX4X4X4X4;
                default:
                    return pcieBifurcateXXX8X4X4;
            }
            break;
        }
        case PCIe::BifurcationMode::hi_x8:
        {
            switch (fallback)
            {
                case pcieBifurcateX4X4X4X4:
                case pcieBifurcateXXX8X4X4:
                    return pcieBifurcateXXX8X4X4;
                default:
                    return pcieBifurcateXXX8XXX8;
            }
            break;
        }
        case PCIe::BifurcationMode::hi_x4x4:
        {
            switch (fallback)
            {
                case pcieBifurcateX4X4X4X4:
                case pcieBifurcateXXX8X4X4:
                    return pcieBifurcateX4X4X4X4;
                default:
                    return pcieBifurcateX4X4XXX8;
            }
            break;
        }

        case PCIe::BifurcationMode::disabled:
            return pcieBifurcateXXXXXXXX;
    }

    log<level::ERR>(
        "Don't know how to deal with requested bifurcation mode, using "
        "fallback",
        entry("VALUE=%s",
              PCIe::convertBifurcationModeToString(bifurcation).c_str()),
        entry("DEFAULT=%d", fallback));
    return fallback;
}

/**
 * @brief Handler of IPMI OEM messages for request PCIe bifurcation
 * configuration.
 */
static ipmi::RspType<std::vector<uint8_t>>
    ipmiOEMGetPCIeBifurcation(ipmi::Context::ptr ctx,
                              std::vector<uint8_t> inputData)
{
    std::vector<uint8_t> result;
    if (inputData.size() % 3)
    {
        return ipmi::responseReqDataLenInvalid();
    }

    DefaultConfiguration defaultCfg;
    uint8_t* buf;
    int len;
    for (buf = inputData.data(), len = inputData.size(); len > 0;
         buf += 3, len -= 3)
    {
        const uint8_t socket = buf[0];
        const uint8_t iouNum = buf[1];
        const uint8_t bif = buf[2];
        defaultCfg.emplace(std::make_pair(socket, iouNum), bif);
    }

    std::string service;
    boost::system::error_code ec =
        ipmi::getService(ctx, PCIe::interface, dbus::pcie_cfg::path, service);
    if (ec)
    {
        log<level::ERR>("Failed to get service",
                        entry("ERROR=%s", ec.message().c_str()),
                        entry("PATH=%s", dbus::pcie_cfg::path),
                        entry("INTERFACE=%s", PCIe::interface));
        ipmi::responseUnspecifiedError();
    }

    BifurcationConfiguration bifurcationConfig;
    ec = ipmi::getDbusProperty<BifurcationConfiguration>(
        ctx, service, dbus::pcie_cfg::path, PCIe::interface,
        dbus::pcie_cfg::properties::bifurcation, bifurcationConfig);
    if (ec)
    {
        log<level::ERR>("Failed to get propriety",
                        entry("ERROR=%s", ec.message().c_str()),
                        entry("PATH=%s", dbus::pcie_cfg::path),
                        entry("INTERFACE=%s", PCIe::interface));
        ipmi::responseUnspecifiedError();
    }
    for (auto [socket, iouNumber, bifurcation] : bifurcationConfig)
    {
        iouNumber -= 1; // iouNumber is 0-based in BIOS, but 1-based in BMC
                        // this is for historical reasons and now can't be
                        // changed without break Manufacturing/QC.
        auto found = defaultCfg.find({socket, iouNumber});
        if (found == defaultCfg.end())
        {
            log<level::ERR>("Unexpected Socket or PCIe port number",
                            entry("SOCKET=%d", socket),
                            entry("PORT=%d", iouNumber));
            continue;
        }
        uint8_t fallback = found->second;
        auto bifurcationCode = convertBifurcation(bifurcation, fallback);
        result.emplace_back(socket);
        result.emplace_back(iouNumber);
        result.emplace_back(bifurcationCode);
    }
    return ipmi::responseSuccess(result);
}

void registerHostConfigHandlers()
{
    log<level::INFO>("Registering OEM handler for Host Configuration");
    ipmi::registerOemHandler(ipmi::prioOpenBmcBase, ianaYadro,
                             cmd::cmdGetPCIeBifurcation, ipmi::Privilege::Admin,
                             ipmiOEMGetPCIeBifurcation);
}

} // namespace host_config
} // namespace yadro
} // namespace ipmi
