#http://username:password@dynupdate.no-ip.com/nic/update?hostname=mytest.example.com&myip=192.0.2.25
# update.py
import requests
import logging
import argparse
import sys

# Set up logging
logging.basicConfig(level=logging.INFO, filename="/var/log/dnshost/dnshost.log", format="%(asctime)s %(levelname)s %(message)s")

def update_noip(username, password, hostname, ip):
    url = f"https://dynupdate.no-ip.com/nic/update?hostname={hostname}&myip={ip}"
    response = requests.get(url, auth=(username, password))
    
    if "nochg" in response.text:
        logging.info("NOIP: Current IP address, no update performed.")
    
    if "good" in response.text:
        logging.info("NOIP: NOIP: IP updated: %s" % response.text)
    
    if "nohost" in response.text:
        logging.info("Hostname does not exist for the specified account.")
    
    if "badauth" in response.text:
        logging.info("NOIP: Invalid username or password.")
    
    if "badagent" in response.text:
        logging.info("NOIP: Client disabled.")
    
    if "abuse" in response.text:
        logging.info("NOIP: Username blocked due to abuse.")
    
    if "911" in response.text:
        logging.info("NOIP: Unexpected error.")
    return response.text

def update_dynudns(username, password, hostname, ip):
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
    return response.text

def update_duckdns(token, hostname, ip):
    url = f"https://www.duckdns.org/update/{token}/{hostname}/{ip}"
    response = requests.get(url)
    return response.text

def update_ydns(username, password, hostname, ip):
    url = f"http://ydns.io/api/v1/update/?host={hostname}"
    headers = {  'accept': 'text/html,application/xhtml+xml,application/xml',
              'user-agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.99 Safari/537.36',
              'credentials': "same-origin"}
    response = requests.get(url, auth=(username, password), data={'ip': ip}, headers=headers)
    if response.status_code == 200:
        logging.info("YDNS: Update successful: %s", response.text)
    elif response.status_code == 400:
        logging.info("YDNS: Error 400: Invalid input parameters.")
    elif response.status_code == 401:
        logging.info("YDNS: Error 401: Authentication issues.")
    elif response.status_code == 404:
        logging.info("YDNS: Error 404: Host not found.")
    else:
        logging.info("YDNS: Unexpected error %s: %s", response.status_code, response.text)
    return response.text

def main():
    parser = argparse.ArgumentParser(description="Update DNS records for No-IP, DynuDNS, DuckDNS, or YDNS.")
    parser.add_argument('-d', '--dns', required=True, choices=['noip', 'dynu', 'duck', 'ydns'], help='DNS service to update')
    parser.add_argument('--username', help='Username for No-IP, DynuDNS, or YDNS')
    parser.add_argument('--password', help='Password for No-IP, DynuDNS, or YDNS')
    parser.add_argument('--token', help='Token for DuckDNS')
    parser.add_argument('--hostname', required=True, help='The hostname to update')
    parser.add_argument('--ip', required=True, help='The IP address to set')

    args = parser.parse_args()

    if args.dns == 'noip':
        if not args.username or not args.password:
            logging.info("Username and password are required for No-IP.")
            sys.exit(1)
        result = update_noip(args.username, args.password, args.hostname, args.ip)

    elif args.dns == 'dynu':
        if not args.username or not args.password:
            logging.info("Username and password are required for DynuDNS.")
            sys.exit(1)
        result = update_dynudns(args.username, args.password, args.hostname, args.ip)

    elif args.dns == 'duck':
        if not args.token:
            logging.info("Token is required for DuckDNS.")
            sys.exit(1)
        result = update_duckdns(args.token, args.hostname, args.ip)

    elif args.dns == 'ydns':
        if not args.username or not args.password:
            logging.info("Username and password are required for YDNS.")
            sys.exit(1)
        result = update_ydns(args.username, args.password, args.hostname, args.ip)

    print(result)

if __name__ == '__main__':
    main()