import lib
from datetime import datetime
from time import sleep
from yaml import load, SafeLoader

if __name__ == '__main__':
    with open('.config.yml') as config_file:
        config = load(config_file, Loader=SafeLoader)
    for meta in config.get('orgs', list()):
        ghapi = lib.GitHubAPI(meta)
        print('EXAMPLE AUTOMATED TOKEN REFRESH USING GITHUB APP AND PYGITHUB')
        print('ctrl-c to exit; otherwise runs infinitely\n')
        while True:
            print(f"- TIMESTAMP: {datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')}")
            print(f"- REPO OBJECT: {ghapi.org_repos()[0].full_name}\n")
            sleep(5)
    print('done')