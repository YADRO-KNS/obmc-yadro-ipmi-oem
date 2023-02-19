// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2023, KNS Group LLC (YADRO)

#include <time.h>

#include <inventory_cmd.hpp>
#include <ipmid/api.hpp>
#include <ipmid/utils.hpp>
#include <oem_cmd.hpp>
#include <phosphor-logging/lg2.hpp>

#include <filesystem>
#include <fstream>

using namespace phosphor::logging;
namespace ipmi
{
namespace yadro
{
namespace inventory
{

/** @brief Register IPMI OEM command handler. */
void registerOEMInventoryHandlers() __attribute__((constructor));

/**
 * @brief Handler of IPMI OEM messages for retrieving inventory BLOB.
 */
template <void (*F)(ipmi::Context::ptr ctx, const std::vector<uint8_t>&)>
static ipmi::RspType<> ipmiBlobHandler(ipmi::Context::ptr ctx,
                                       ipmi::message::Payload& data)
{
    static std::vector<uint8_t> blob;
    try
    {
        // first byte: session state
        uint8_t state;
        if (data.unpack(state))
        {
            return ipmi::responseReqDataLenInvalid();
        }
        if (state != IPMI_SESSION_BEGIN && state != IPMI_SESSION_DATA &&
            state != IPMI_SESSION_COMMIT)
        {
            throw std::runtime_error("Invalid BLOB session state");
        }

        // start new session
        if (state == IPMI_SESSION_BEGIN)
        {
            blob.clear();
        }

        // append next part of data to BLOB
        if (!data.fullyUnpacked())
        {
            std::vector<uint8_t> chunk;
            data.unpack(chunk);
            blob.insert(blob.end(), chunk.begin(), chunk.end());
        }

        // commit BLOB
        if (state == IPMI_SESSION_COMMIT)
        {
            if (blob.empty())
            {
                throw std::runtime_error("Inventory BLOB is empty");
            }
            F(ctx, blob);
            blob.clear();
            log<level::INFO>("Inventory BLOB handled successfully");
        }
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>("Unable to handle inventory BLOB",
                        entry("ERROR=%s", ex.what()));
        return ipmi::responseUnspecifiedError();
    }

    return ipmi::responseSuccess();
}

/**
 * @brief Handler of SMBIOS dump.
 */
static void handleSmbios(ipmi::Context::ptr ctx,
                         const std::vector<uint8_t>& data)
{
    using namespace smbios;
    // save dump in MDR format
    const std::filesystem::path mdrPath(mdrType2File);
    std::filesystem::create_directories(mdrPath.parent_path());

    std::ofstream mdrFile(mdrType2File, std::ios::trunc | std::ios::binary);
    if (mdrFile.is_open())
    {
        time_t timestamp;
        time(&timestamp);
        const MDRSMBIOSHeader header = {mdr2Version, mdrTypeII,
                                        static_cast<uint32_t>(timestamp),
                                        static_cast<uint32_t>(data.size())};
        mdrFile.write(reinterpret_cast<const char*>(&header), sizeof(header));
        mdrFile.write(reinterpret_cast<const char*>(data.data()), data.size());
        mdrFile.close();
    }
    if (!mdrFile.good())
    {
        throw std::runtime_error("Unable to write MDR file");
    }

    // call MDR parser
    std::string service;
    if (ipmi::getService(ctx, mdrv2Interface, mdrv2Path, service))
    {
        throw std::runtime_error("failed to get MDR parser service");
    }
    ctx->bus->async_method_call(
        [](boost::system::error_code ec) {
            if (ec)
            {
                log<level::ERR>("Unable to call MDR parser",
                                entry("ERROR=%s", ec.message().c_str()));
            }
        },
        service.c_str(), mdrv2Path, mdrv2Interface, "AgentSynchronizeData");
}

/**
 * @brief Handler of Storage list.
 */
static void handleStorages(ipmi::Context::ptr ctx,
                           const std::vector<uint8_t>& data)
{
    using namespace storage;

    // save dump in CSV format
    const std::filesystem::path dataPath(storageDataFile);
    std::filesystem::create_directories(dataPath.parent_path());

    std::ofstream dataFile(storageDataFile, std::ios::trunc);
    if (!dataFile.is_open())
    {
        throw std::runtime_error("Unable to open storage data file");
    }
    const size_t storageCount = data.size() / sizeof(Storage);
    static constexpr const char* sep = ";";
    for (size_t i = 0; i < storageCount; ++i)
    {
        const Storage* st =
            reinterpret_cast<const Storage*>(&data.at(i * sizeof(Storage)));
        const std::string model = st->model;
        const std::string serial = st->serial;
        const std::string path = st->path;
        const std::string sizeBytes = std::to_string(st->size);

        std::string iface;
        switch (st->iface)
        {
            case STORAGE_IFACE_SATA:
                iface = "SATA";
                break;
            case STORAGE_IFACE_NVME:
                iface = "NVMe";
                break;
            case STORAGE_IFACE_SCSI:
                // we assume that only SAS drives would be used
                iface = "SAS";
                break;
        }
        std::string type;
        if (st->rotationRate == 1)
        {
            type = "SSD";
        }
        else if (st->rotationRate > 1)
        {
            type = "HDD";
        }
        std::string vendor;
        if (st->vendorId)
        {
            std::ostringstream vid;
            vid << std::hex << std::setw(4) << std::setfill('0')
                << st->vendorId;
            vendor = vid.str();
        }

        dataFile << path << sep << iface << sep << type << sep << vendor << sep
                 << model << sep << serial << sep << sizeBytes << std::endl;
    }
    dataFile.close();
    if (!dataFile.good())
    {
        throw std::runtime_error("Unable to write storage data file");
    }

    // call storage parser
    std::string service;
    if (ipmi::getService(ctx, storageMgrInterface, storageMgrPath, service))
    {
        throw std::runtime_error("failed to get storage manager service");
    }
    ctx->bus->async_method_call(
        [](boost::system::error_code ec) {
            if (ec)
            {
                log<level::ERR>("Unable to call storage manager parser",
                                entry("ERROR=%s", ec.message().c_str()));
            }
        },
        service.c_str(), storageMgrPath, storageMgrInterface, "Rescan");
}

void registerOEMInventoryHandlers()
{
    log<level::INFO>("Registering OEM handler for Inventory data");
    ipmi::registerOemHandler(ipmi::prioOpenBmcBase, ianaYadro, cmd::cmdSmbios,
                             ipmi::Privilege::Admin,
                             ipmiBlobHandler<handleSmbios>);
    ipmi::registerOemHandler(ipmi::prioOpenBmcBase, ianaYadro, cmd::cmdStorage,
                             ipmi::Privilege::Admin,
                             ipmiBlobHandler<handleStorages>);
}

} // namespace inventory
} // namespace yadro
} // namespace ipmi
