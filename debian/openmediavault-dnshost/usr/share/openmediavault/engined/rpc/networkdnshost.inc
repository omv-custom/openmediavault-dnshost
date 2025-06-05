<?php

class NetworkDnshost extends \OMV\Rpc\ServiceAbstract 
{
    private const IP_CHECK_URL = 'https://api.ipify.org';
    private const PROVIDERS = ['noip', 'ddns', 'ydns', 'freedns'];
    
    public function getName()
    {
        return "NetworkDnshost";
    }

    public function initialize()
    {
        $this->registerMethod('getSettings');
        $this->registerMethod('setSettings');
        $this->registerMethod('updateIP');
        $this->registerMethod('doFix');
        $this->registerMethod('testConnection');
    }

    function getSettings($params, $context) {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
    
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.system.network.dnshost');
        $cron = $db->get('conf.system.cron.job', '32664b22-5a9d-11ec-8834-6f00f75b23dd');
    
        return $object->getAssoc();
    }

    public function setSettings($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get('conf.system.network.dnshost');
        $object->setAssoc($params);
        $db->set($object);
    
        return $object->getAssoc();
    }
     
    private function updateSettingsActualIp()
    {
        try {
            $cmd = new \OMV\System\Process(['curl', '-s', self::IP_CHECK_URL]);
            $cmd->setRedirect2to1();
            $cmd->execute($output);
            
            $currentIp = implode("\n", $output);
            if (!filter_var($currentIp, FILTER_VALIDATE_IP)) {
                throw new \RuntimeException("Invalid IP address received");
            }
            
            $params = array_reduce(
                self::PROVIDERS,
                fn($carry, $provider) => $carry + ["{$provider}_ip" => $currentIp],
                []
            );
            
            $db = \OMV\Config\Database::getInstance();
            $object = $db->get('conf.system.network.dnshost');
            $object->setAssoc($params);
            $db->set($object);
            
        } catch (\Exception $e) {
            \OMV\System\LogFileSpec::getByName('dnshost')->log(
                sprintf("IP update failed: %s", $e->getMessage())
            );
        }
    }
    
    public function updateIP($params)
    {
        return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename) use ($params) {
            $provider = $params['provider'] ?? '';
            
            if (!in_array($provider, self::PROVIDERS)) {
                throw new \OMV\Exception("Invalid provider specified");
            }
            
            $db = \OMV\Config\Database::getInstance();
            $object = $db->get('conf.system.network.dnshost');
            
            if (!$object->get($provider)) {
                throw new \OMV\Exception("Service is disabled for provider: {$provider}");
            }
            
            $scriptPath = sprintf(
                '/usr/sbin/dynamic-hostname.py -d %s',
                escapeshellarg($provider)
            );
            
            $cmd = new \OMV\System\Process(['python3', $scriptPath]);
            $cmd->setRedirect2to1();
            
            $exitStatus = $this->exec($cmd->getCommandLine(), $output, $bgOutputFilename);
            
            if (0 !== $exitStatus) {
                throw new \OMV\ExecException($cmd->getCommandLine(), $output, $exitStatus);
            }
            
            return $output;
        });
    }
    
    public function doFix($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);

        return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename) {
            $cmd = new \OMV\System\Process('/usr/sbin/omv-dnshost-fix-cron');
            $cmd->setRedirect2to1(true);
            
            if (0 !== $this->exec($cmd->getCommandLine(), $output, $bgOutputFilename)) {
                throw new \OMV\ExecException($cmd->getCommandLine(), $output);
            }
            
            return $output;
        });
    }

public function testConnection($params, $context) {
    $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
    
    // Validate provider parameter
    $providers = ['noip', 'ddns', 'ydns', 'freedns'];
    if (!isset($params['provider']) || !in_array($params['provider'], $providers)) {
        throw new \OMV\Exception("Invalid provider specified. Valid providers: %s",
            implode(', ', $providers));
    }
    $provider = $params['provider'];
    
    return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename) use ($provider) {
        $cmd = new \OMV\System\Process([
            'python3',
            '/usr/sbin/dynamic-hostname.py',
            '-d', $provider,
            '--test'
        ]);
        $cmd->setRedirect2to1();
        
        // Execute and capture output
        $output = [];
        $exitStatus = $this->exec($cmd->getCommandLine(), $output, $bgOutputFilename);
        
        // Parse output
        $outputStr = implode("\n", $output);
        if (0 !== $exitStatus) {
            if (strpos($outputStr, "Connection test failed") !== false) {
                throw new \OMV\Exception($outputStr);
            }
            throw new \OMV\ExecException($cmd->getCommandLine(), $output, $exitStatus);
        }
        
        // Check for success message
        if (strpos($outputStr, "Success:") === false) {
            throw new \OMV\Exception("Connection test failed: %s", $outputStr);
        }
        
        return [
            "provider" => $provider,
            "message" => trim(str_replace("Success:", "", $outputStr)),
            "exitStatus" => $exitStatus
        ];
    });
}

}
