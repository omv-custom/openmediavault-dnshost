<?php

class NetworkDnshost extends \OMV\Rpc\ServiceAbstract {
	public function getName() {
		return "NetworkDnshost";
	}

	public function initialize() {
        $this->registerMethod('getSettings');
        $this->registerMethod('setSettings');
        $this->registerMethod('updateIP_noip');
        $this->registerMethod('updateIP_dynuddns');
	}

	function getSettings($params, $context) {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, [ 'role' => OMV_ROLE_ADMINISTRATOR ]);
         
        $this->updateSettingsActualIp();
        
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.system.network.dnshost');
        return $object->getAssoc();
	}

	function setSettings($params, $context) {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.system.network.dnshost');
        $object->setAssoc($params);
        // Set the configuration object.
        $db->set($object);
        // Return the configuration object.
        
        #$this->updateIP_noip();
        #$this->updateIP_dynuddns();
        
        return $object->getAssoc();
	}
     
    function updateSettingsActualIp()
    {
        //wget -q -O- http://ipecho.net/plain
        
        //GET ACTUAL IP ADDRESS
         $cmd = new \OMV\System\Process("wget", " -q -O- http://ipecho.net/plain");
         $cmd->setRedirect2to1();
         $cmd->execute($output);
         $stats = implode("\n", $output);
        //END
        
        $db = \OMV\Config\Database::getInstance();
        $params['noip_ip'] = $stats;
        $params['ddns_ip'] = $stats;
        $object = $db->get('conf.system.network.dnshost');
        $object->setAssoc($params);
        $db->set($object);
    }
    
    function updateIP_noip() {
        return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($object) {
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.system.network.dnshost');
        if (FALSE === $object->get("noip")) {
              throw new \OMV\Exception(
				"Failed to run scheduled job because the service is disabled.");
        } else {
            $cmdArgs = [];
			$cmdArgs[] = "--shell";
			$cmdArgs[] = "--non-interactive";
			$cmdArgs[] = "--";
			$cmdArgs[] = build_path(DIRECTORY_SEPARATOR, \OMV\Environment::get("OMV_CRONSCRIPTS_DIR"), sprintf("noip-%s", 100));
             
             $username = $object->get("noip_username");
             $password = $object->get("noip_password");
             $hostname = $object->get("noip_hostname");
             $ip = $object->get("noip_ip");
             $cmd = new \OMV\System\Process("python3 /usr/sbin/noip-update.py", " --username $username --password $password --hostname $hostname --ip $ip");
             $cmd->setRedirect2to1();
             $cmd->execute($output);
             $cmdLine = $cmd->getCommandLine();
             $stats = implode("\n", $output);
            
             $exitStatus = $this->exec($cmdLine, $output, $bgOutputFilename);
             
             if (0 !== $exitStatus)
				throw new \OMV\ExecException($cmdLine, $output, $exitStatus);
            
            return $output;
        }

		});
    }
    
    function updateIP_dynuddns() {
        return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($object) {
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.system.network.dnshost');
        if (FALSE === $object->get("ddns")) {
              throw new \OMV\Exception(
				"Failed to run scheduled job because the service is disabled.");
        } else {
            $cmdArgs = [];
			$cmdArgs[] = "--shell";
			$cmdArgs[] = "--non-interactive";
			$cmdArgs[] = "--";
			$cmdArgs[] = build_path(DIRECTORY_SEPARATOR, \OMV\Environment::get("OMV_CRONSCRIPTS_DIR"), sprintf("ddns-%s", 100));
             
             $username = $object->get("ddns_username");
             $password = $object->get("ddns_password");
             $hostname = $object->get("ddns_hostname");
             $ip = $object->get("ddns_ip");
             $cmd = new \OMV\System\Process("python3 /usr/sbin/dynuddns-update.py", " --username $username --password $password --hostname $hostname --ip $ip");
             $cmd->setRedirect2to1();
             $cmd->execute($output);
             $cmdLine = $cmd->getCommandLine();
             $stats = implode("\n", $output);
            
             $exitStatus = $this->exec($cmdLine, $output, $bgOutputFilename);
             
             if (0 !== $exitStatus)
				throw new \OMV\ExecException($cmdLine, $output, $exitStatus);
            
            return $output;
        }

		});
    }

}
