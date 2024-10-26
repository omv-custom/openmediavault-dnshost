#http://username:password@dynupdate.no-ip.com/nic/update?hostname=mytest.example.com&myip=192.0.2.25
# update.py
import requests
import logging
import argparse

# Set up logging
logging.basicConfig(level=logging.INFO)

def update_noip_hostname(username, password, hostname, ip):
    url = f"https://dynupdate.no-ip.com/nic/update?hostname={hostname}&myip={ip}"
    response = requests.get(url, auth=(username, password))
    
    if "nochg" in response.text:
        logging.info("Current IP address, no update performed.")
    
    if "good" in response.text:
        logging.info("IP updated: %s" % response.text)
    
    if "nohost" in response.text:
        logging.info("Hostname does not exist for the specified account.")
    
    if "badauth" in response.text:
        logging.info("Invalid username or password.")
    
    if "badagent" in response.text:
        logging.info("Client disabled.")
    
    if "abuse" in response.text:
        logging.info("Username blocked due to abuse.")
    
    if "911" in response.text:
        logging.info("Unexpected error.")
    
    print(response.text)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Update No-IP hostname IP address.')
    parser.add_argument('--username', required=True, help='No-IP account username')
    parser.add_argument('--password', required=True, help='No-IP account password')
    parser.add_argument('--hostname', required=True, help='No-IP hostname to update')
    parser.add_argument('--ip', required=True, help='IP address to set')
    
    args = parser.parse_args()
    
    update_noip_hostname(args.username, args.password, args.hostname, args.ip)