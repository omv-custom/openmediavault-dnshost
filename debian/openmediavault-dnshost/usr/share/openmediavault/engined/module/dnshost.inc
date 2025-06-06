<?php

/**
 * Copyright (C) 2013-2024 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

class OMVModuleDnshost extends \OMV\Engine\Module\ServiceAbstract implements \OMV\Engine\Module\IServiceStatus
{
    public function getName()
    {
        return "dnshosts";
    }
    
    public function getStatus()
    {
        return array(
            "name" => "DnsHost",
            "title" => "DnsHost",
            "enabled" => true,
            "running" => true
        );
    }

    public function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher)
    {
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            "org.openmediavault.conf.network.dnshost",
            [ $this, "setDirty" ]
        );
    }
}
