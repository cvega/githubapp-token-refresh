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
            self.github_client() if not self.gh or self.is_expired() else None
            return func(self, *args, **kwargs)
        return wrapper

    def is_expired(self):
        return datetime.now().timestamp() + 60 >= self.expires_at.timestamp()

    def token(self):
        kwargs = {
            "integration_id": self.meta.get("app_id"),
            "private_key": self.private_key,
        }
        if self.meta.get("base_url"):
            kwargs["base_url"] = self.meta.get("base_url")
        integration = GithubIntegration(
            **kwargs,
        )
        token = integration.get_access_token(self.meta.get("installation_id"))
        return token

    def github_client(self):
        self._auth = self.token()
        kwargs = {"login_or_token": self._auth.token}
        if self.meta.get("base_url"):
            kwargs["base_url"] = self.meta.get("base_url")
        self.gh = Github(**kwargs)
        self.expires_at = self._auth.expires_at.replace(tzinfo=timezone.utc)


class GitHubAPI(GitHubAPIAuth):
    @GitHubAPIAuth.auth
    def org_repos(self):
        self.org = self.gh.get_organization(self.meta.get("name"))
        return self.org.get_repos(type="private")
