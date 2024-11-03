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
        $this->registerMethod('updateIP_ydns');
        $this->registerMethod('updateIP_freedns');
        $this->registerMethod('doFix');
	}

	function getSettings($params, $context) {
        $this->validateMethodContext($context, [ 'role' => OMV_ROLE_ADMINISTRATOR ]);
         
        $this->updateSettingsActualIp();
        
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.system.network.dnshost');
        return $object->getAssoc();
	}

	function setSettings($params, $context) {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.system.network.dnshost');
        $object->setAssoc($params);
        $db->set($object);
    
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
        $params['ydns_ip'] = $stats;
        $params['freedns_ip'] = $stats;
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
             $cmd = new \OMV\System\Process("python3 /usr/sbin/dynamic-hostname.py", "-d noip");
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
             $cmd = new \OMV\System\Process("python3 /usr/sbin/dynamic-hostname.py", "-d dynu");
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
    
    function updateIP_ydns() {
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
             
             $username = $object->get("ydns_username");
             $password = $object->get("ydns_password");
             $hostname = $object->get("ydns_hostname");
             $ip = $object->get("ydns_ip");
             $cmd = new \OMV\System\Process("python3 /usr/sbin/dynamic-hostname.py", "-d ydns");
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
    
    function updateIP_freedns() {
        return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($object) {
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.system.network.dnshost');
        if (FALSE === $object->get("freedns_enabled")) {
              throw new \OMV\Exception(
				"Failed to run scheduled job because the service is disabled.");
        } else {
            $cmdArgs = [];
			$cmdArgs[] = "--shell";
			$cmdArgs[] = "--non-interactive";
			$cmdArgs[] = "--";
			$cmdArgs[] = build_path(DIRECTORY_SEPARATOR, \OMV\Environment::get("OMV_CRONSCRIPTS_DIR"), sprintf("ddns-%s", 100));
             
             $username = $object->get("freedns_username");
             $password = $object->get("freedns_password");
             $hostname = $object->get("freedns_hostname");
             $ip = $object->get("freedns_ip");
             $cmd = new \OMV\System\Process("python3 /usr/sbin/dynamic-hostname.py", "-d freedns");
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
    
# CRON JOBS FIX
    public function doFix($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);

        // Create the background process.
        return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
            use ($params) {
                $cmd = new \OMV\System\Process('/usr/sbin/omv-dnshost-fix-cron');
                $cmd->setRedirect2to1(true);
                $cmdLine = $cmd->getCommandLine();
                if (0 !== $this->exec($cmdLine, $output, $bgOutputFilename))
                    throw new \OMV\ExecException($cmdLine, $output);
                return $output;
            }
        );
    }

}
