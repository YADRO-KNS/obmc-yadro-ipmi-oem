// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2023, KNS Group LLC (YADRO)

#pragma once

#include <ipmid/api.hpp>
#include <xyz/openbmc_project/Control/PCIe/server.hpp>

#include <variant>

namespace ipmi
{
namespace yadro
{
namespace host_config
{

namespace dbus
{
namespace pcie_cfg
{
constexpr const char* path = "/xyz/openbmc_project/control/host0/pcie";
namespace properties
{
constexpr const char* bifurcation = "Bifurcation";
} // namespace properties
} // namespace pcie_cfg
} // namespace dbus

using namespace sdbusplus::xyz::openbmc_project::Control::server;

using BifurcationConfiguration =
    std::vector<std::tuple<uint8_t, uint8_t, PCIe::BifurcationMode>>;
using DbusPropVariant =
    std::variant<std::string, uint32_t, bool, double, BifurcationConfiguration>;

// PCIe Bifurcation mode
static constexpr uint8_t pcieBifurcateX4X4X4X4 = 0;
static constexpr uint8_t pcieBifurcateX4X4XXX8 = 1;
static constexpr uint8_t pcieBifurcateXXX8X4X4 = 2;
static constexpr uint8_t pcieBifurcateXXX8XXX8 = 3;
static constexpr uint8_t pcieBifurcateXXXXXX16 = 4;
static constexpr uint8_t pcieBifurcateXXXXXXXX = 0xF;

// PCIe Bifurcation modes list (Socket, IouNumber) -> Bifurcation
using DefaultConfiguration = std::map<std::tuple<uint8_t, uint8_t>, uint8_t>;

} // namespace host_config
} // namespace yadro
} // namespace ipmi
