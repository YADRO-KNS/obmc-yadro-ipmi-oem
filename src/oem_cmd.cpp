// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2022, KNS Group LLC (YADRO)

#include <inventory_cmd.hpp>
#include <oem_cmd.hpp>

namespace ipmi
{

/**
 * @brief The base function to library initialize.
 *        The constructor registries IPMI command handlers
 *
 */
void registerOEMHandlers(void) __attribute__((constructor));

void registerOEMHandlers(void)
{}

} // namespace ipmi
