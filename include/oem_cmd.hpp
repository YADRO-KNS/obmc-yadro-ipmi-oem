// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2023, KNS Group LLC (YADRO)

#pragma once

#include <ipmid/api-types.hpp>
#include <user_channel/user_layer.hpp>

#include <string>

namespace ipmi
{
namespace yadro
{

/** @brief IANA number of YADRO. */
static constexpr uint32_t ianaYadro = 49769;

namespace cmd
{
/** @brief Command number for SMBIOS dump. */
static constexpr ipmi::Cmd cmdSmbios = 0xa0;
/** @brief Command number for Storage list. */
static constexpr ipmi::Cmd cmdStorage = 0xa1;

/** @brief Command number for get PCIe Bifurcation config. */
static constexpr ipmi::Cmd cmdGetPCIeBifurcation = 0xb0;

} // namespace cmd

} // namespace yadro

} // namespace ipmi
