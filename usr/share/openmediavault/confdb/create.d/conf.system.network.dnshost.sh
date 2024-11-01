#!/bin/sh
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
# @copyright Copyright (c) 2009-2013 Volker Theile
# @copyright Copyright (c) 2013-2024 openmediavault plugin developers
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

set -e

. /usr/share/openmediavault/scripts/helper-functions

SERVICE_XPATH_NAME="dnshosts"
SERVICE_XPATH="/config/system/network/${SERVICE_XPATH_NAME}"

if ! omv_config_exists "${SERVICE_XPATH}"; then
    ip="$(wget -q -O- http://ipecho.net/plain)"
    omv_config_add_node "/config/system/network" "${SERVICE_XPATH_NAME}"
    omv_config_add_key "${SERVICE_XPATH}" "enable" "0"
    omv_config_add_key "${SERVICE_XPATH}" "noip" "false"
    omv_config_add_key "${SERVICE_XPATH}" "ddns" "false"
    omv_config_add_key "${SERVICE_XPATH}" "ydns" "false"
    omv_config_add_key "${SERVICE_XPATH}" "freedns_enabled" "false"
    omv_config_add_key "${SERVICE_XPATH}" "noip_username" "username"
    omv_config_add_key "${SERVICE_XPATH}" "noip_password" "password"
    omv_config_add_key "${SERVICE_XPATH}" "noip_hostname" "hostname"
    omv_config_add_key "${SERVICE_XPATH}" "noip_ip" "${ip}"
    
    omv_config_add_key "${SERVICE_XPATH}" "ddns_username" "username"
    omv_config_add_key "${SERVICE_XPATH}" "ddns_password" "password"
    omv_config_add_key "${SERVICE_XPATH}" "ddns_hostname" "hostname"
    omv_config_add_key "${SERVICE_XPATH}" "ddns_ip" "${ip}"
    
    omv_config_add_key "${SERVICE_XPATH}" "ydns_username" "username"
    omv_config_add_key "${SERVICE_XPATH}" "ydns_password" "password"
    omv_config_add_key "${SERVICE_XPATH}" "ydns_hostname" "hostname"
    omv_config_add_key "${SERVICE_XPATH}" "ydns_ip" "${ip}"
    omv_config_add_key "${SERVICE_XPATH}" "cron" "false"
    
    omv_config_add_key "${SERVICE_XPATH}" "freedns_username" "username"
    omv_config_add_key "${SERVICE_XPATH}" "freedns_password" "password"
    omv_config_add_key "${SERVICE_XPATH}" "freedns_hostname" "hostname"
    omv_config_add_key "${SERVICE_XPATH}" "freedns_ip" "${ip}"
fi

# add or re-add cron job
omv-dnshost-fix-cron

exit 0
