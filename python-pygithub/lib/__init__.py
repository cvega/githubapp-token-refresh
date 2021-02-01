from datetime import datetime, timezone
from github import Github, GithubIntegration


class GitHubAPIAuth(object):
    def __init__(self, meta):
        self.meta = meta
        self._auth, self.expires_at, self.gh = None, None, None
        with open(self.meta["pem"]) as private_key:
            self.private_key = private_key.read()

    @staticmethod
    def auth(func):
        def wrapper(self, *args, **kwargs):
            if not self.gh or self.is_expired():
                print("- TOKEN STATUS: EXPIRED")
                self.github_client()
                print("- TOKEN STATUS: REFRESHED")
            print("- TOKEN STATUS: CURRENT")
            print(f"- TOKEN EXPIRY: {self.expires_at.strftime('%Y-%m-%d %H:%M:%S')}")

            return func(self, *args, **kwargs)

        return wrapper

    def is_expired(self):
        return datetime.now().timestamp() + 60 >= self.expires_at.timestamp()

    def token(self):
        return GithubIntegration(
            self.meta.get("app_id"), self.private_key
        ).get_access_token(self.meta.get("installation_id"))

    def github_client(self):
        self._auth = self.token()
        self.gh = Github(self._auth.token)
        self.expires_at = self._auth.expires_at.replace(tzinfo=timezone.utc)


class GitHubAPI(GitHubAPIAuth):
    @GitHubAPIAuth.auth
    def org_repos(self):
        self.org = self.gh.get_organization(self.meta.get("name"))
        return self.org.get_repos(type="private")
