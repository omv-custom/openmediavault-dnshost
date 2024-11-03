#!/usr/bin/env python3

import requests
import logging
import argparse
import sys
import subprocess
import json
import os

# Set up logging
logging.basicConfig(level=logging.INFO, filename="/var/log/dnshost/dnshost.log", format="%(asctime)s %(levelname)s %(message)s", datefmt="%Y-%m-%d %H:%M:%S")

def omv_config_get():
    try:
        result = subprocess.run(['omv-confdbadm', 'read', 'conf.system.network.dnshost'], capture_output=True, text=True, check=True)
        return json.loads(result.stdout)
    except subprocess.CalledProcessError as e:
        logging.error(f"Error getting config: {e}")
        return None
    except json.JSONDecodeError as e:
        logging.error(f"Error decoding JSON: {e}")
        return None

def omv_config_update(config):
    try:
        json_config = json.dumps(config)
        subprocess.run(['omv-confdbadm', 'update', 'conf.system.network.dnshost', json_config], check=True)
        return True
    except subprocess.CalledProcessError as e:
        logging.error(f"Error updating config: {e}")
        return False

def update_dns(provider):
    config = omv_config_get()

    username = config.get(f"{provider}_username")
    password = config.get(f"{provider}_password")
    hostname = config.get(f"{provider}_hostname")

    if not all([username, password, hostname]):
        logging.error(f"{provider.upper()}: Missing configuration")
        return "configuration error", 0

    try:
        ip = requests.get('https://api.ipify.org', timeout=30).text
        logging.info(f"Detected IP: {ip}")
    except Exception as e:
        logging.error(f"Could not retrieve current IP: {e}")
        return str(e), 0

    provider_urls = {
        'noip': f"https://dynupdate.no-ip.com/nic/update?hostname={hostname}&myip={ip}",
        'ddns': f"https://api.dynu.com/nic/update?hostname={hostname}&myip={ip}&password={password}",
        'ydns': f"http://ydns.io/api/v1/update/?host={hostname}",
        'freedns': f"http://{username}:{password}@freedns.afraid.org/nic/update?hostname={hostname}&myip={ip}"
    }

    headers = {
        'User-Agent': 'OpenMediaVault-DynamicDNS/1.0',
        'Accept': 'text/html,application/xhtml+xml,application/xml'
    }

    try:
        if provider == 'ydns':
            response = requests.get(provider_urls[provider], auth=(username, password), headers=headers, data={'ip': ip}, timeout=30)
        else:
            response = requests.get(provider_urls[provider], auth=(username, password), headers=headers, timeout=30)

        content = response.text.strip()
        status_code = response.status_code

        if status_code == 200 or "good" in content.lower():
            config[f"{provider}_ip"] = ip
            omv_config_update(config)

        status_messages = {
            "nochg": f"{provider.upper()}: IP unchanged",
            "good": f"{provider.upper()}: IP updated successfully",
            "nohost": f"{provider.upper()}: Hostname not found",
            "badauth": f"{provider.upper()}: Authentication failed",
            "abuse": f"{provider.upper()}: Account abuse detected",
            "911": f"{provider.upper()}: Provider service error"
        }

        for key, message in status_messages.items():
            if key in content.lower():
                logging.info(message)
                return message, status_code

        if status_code == 200:
            message = f"{provider.upper()}: Update successful"
            logging.info(message)
            return message, status_code
        else:
            message = f"{provider.upper()}: Unexpected response - {content}"
            logging.warning(message)
            return message, status_code

    except requests.RequestException as e:
        message = f"{provider.upper()}: Request failed - {str(e)}"
        logging.error(message)
        return message, 0

def main():
    parser = argparse.ArgumentParser(description="Update DNS records for various providers")
    parser.add_argument('-d', '--dns', required=True, choices=['noip', 'ddns', 'ydns', 'freedns'], help='DNS service to update')
    args = parser.parse_args()

    result, status_code = update_dns(args.dns)
    
    if status_code == 200 or "good" in str(result).lower():
        print(f"Successfully updated {args.dns}")
        sys.exit(0)
    else:
        print(f"Update failed for {args.dns}: {result}")
        sys.exit(1)

if __name__ == '__main__':
    if os.geteuid() != 0:
        print("This script must be run as root")
        sys.exit(1)
    main()
