#http://username:password@dynupdate.no-ip.com/nic/update?hostname=mytest.example.com&myip=192.0.2.25
# update.py
import requests
import logging
import argparse

# Set up logging
logging.basicConfig(level=logging.INFO, filename="/var/log/dnshost/dnshost.log", format="%(asctime)s %(levelname)s %(message)s")

def update_ddns_hostname(username, password, hostname, ip):
    url = f"https://api.dynu.com/nic/update?hostname={hostname}&myip={ip}&password={password}"
    response = requests.get(url, auth=(username, password))
    
    if "nochg" in response.text:
        logging.info("DynuDns: Current IP address, no update performed.")
    
    if "good" in response.text:
        logging.info("DynuDns: IP updated: %s" % response.text)
    
    if "nohost" in response.text:
        logging.info("DynuDns: Hostname does not exist for the specified account.")
    
    if "badauth" in response.text:
        logging.info("DynuDns: Invalid username or password.")
    
    if "badagent" in response.text:
        logging.info("DynuDns: Client disabled.")
    
    if "abuse" in response.text:
        logging.info("DynuDns: Username blocked due to abuse.")
    
    if "911" in response.text:
        logging.info("DynuDns: Unexpected error.")
    
    print(response.text)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Update DynuDns hostname IP address.')
    parser.add_argument('--username', required=True, help='DynuDns account username')
    parser.add_argument('--password', required=True, help='DynuDns account password')
    parser.add_argument('--hostname', required=True, help='DynuDns hostname to update')
    parser.add_argument('--ip', required=True, help='IP address to set')
    
    args = parser.parse_args()
    
    update_ddns_hostname(args.username, args.password, args.hostname, args.ip)