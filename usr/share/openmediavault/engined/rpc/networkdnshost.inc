<?php

class NetworkDnshost extends \OMV\Rpc\ServiceAbstract {
	public function getName() {
		return "NetworkDnshost";
	}

	public function initialize() {
        $this->registerMethod('getSettings');
        $this->registerMethod('setSettings');
        $this->registerMethod('updateIP');
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
    
    function updateIP($params) {
        return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($object, $params) {
        $db = \OMV\Config\Database::getInstance();
        $provider = $params['provider'];
        $object = $db->get('conf.system.network.dnshost');
        if (FALSE === $object->get("".$provider."") ) {
              throw new \OMV\Exception(
				"Failed to run scheduled job because the service is disabled.");
        } else {
            $cmdArgs = [];
			$cmdArgs[] = "--shell";
			$cmdArgs[] = "--non-interactive";
			$cmdArgs[] = "--";
			$cmdArgs[] = build_path(DIRECTORY_SEPARATOR, \OMV\Environment::get("OMV_CRONSCRIPTS_DIR"), sprintf("$provider-%s", 100));

             $cmd = new \OMV\System\Process("python3 /usr/sbin/dynamic-hostname.py", "-d $provider");
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
