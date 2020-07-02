# :factory: :shower: Automatically refreshing GitHub App tokens
#### using Octokit.rb and PyGitHub :octocat: (and more to follow soon!)

## Background
Many services and vendors including GitHub provide simple authentication methods in particular a personal access token or PAT. This enables users to chain tooling together using a token that belongs to them or through the creation of a service account. While the PAT enables users to interact with services via the API, it widens potential attack surfaces should the PAT become compromised. GitHub App is focusing on removing the need for service accounts and providing a more secure environment that is easier for people and teams to manage.

### Service accounts on GitHub

- Service accounts are real users with interactive login capabilities.
- Users require a seat on GitHub 
- Rotating account passwords can be difficult if mechanisms to facilitate rotation (e.g. Microsoft Key Vault or Hashicorp Vault) do not exist. This includes updating the consumers of the key.
- You must generate a static password and a token for the service account which is really two passwords now.
- It's not uncommon for a service account to get assigned elevated privileges.

## GitHub App

GitHub App enables organizations, repos, and users to provide a standard and secure interface to the API in relation to their GitHub data. GitHub tries to ease some woes mentioned above by enabling access to the API with no service account using the Principle of Least Privilege and couped with short-lived tokens. This document doesn’t cover how to set a GitHub App up, but it’s fairly simple to create. The common question for many is if no user exists, and no static token exists, how do we keep our short-lived tokens fresh? First, we need to to understand how this process works. Once you’ve created and installed a GitHub App you will have in your possession three pieces of required data. A PEM certificate, an `app_id`, and an `installation_id`. Armed with these pieces of data, we can craft a short-lived JWT token and make a secondary request for an access token to the API. The access token we’re provided is also short lived (1 HR).

### GitHub App Documentation
[GitHub API v3 - Creating a GitHub App](https://developer.github.com/apps/building-github-apps/creating-a-github-app/)

[GitHub API v3 - Authenticating with GitHub Apps](https://developer.github.com/apps/building-github-apps/authenticating-with-github-apps/)

## Why not OAuth App?

OAuth provides `client_id` and `client_secret` (password) and Octokit/GitHub provide a `.check_token` API attribute. This makes validation very simple, but comes with the cost of an additional HTTP request for validation. GitHub App allows us to circumvent this issue by relying on the `expires_at` key provided by the client response. Ultimately, less API requests are better for everyone. It is worth noting that checking a token using OAuth does not count against the GitHub API rate limit.

### OAuth Documentation
[GitHub API v3 - Authorizing OAuth Apps](https://developer.github.com/apps/building-oauth-apps/authorizing-oauth-apps/)

### Octokit.rb `check_token` API Attribute (HTTP Post)
[oauth_applications.rb#L31](https://github.com/octokit/octokit.rb/blob/4ab6bb3f5e5a5a5400f21cc7b915a43e3883afc8/lib/octokit/client/oauth_applications.rb#L31)


## GitHub example.*

The security and overall cost (seat costs, time spent configuring service accounts) benefits are fairly hard to deny but aren't entirely free either because changes to code are required. In this example I try to minimize the required code needed for a fully automated working example. It's not too far off from using Octokit.rb with a PAT. 

In the examples, a single request to the `org_repos` endpoint gets made once every 15 minutes. We let the script run for over an hour to show the behavior.

The example files use a simple `YAML` configuration file which is also included as `.config.yaml` in this repo.

```bash
 ./example.* --sleep 900
EXAMPLE AUTOMATED TOKEN REFRESH USING GITHUB APP AND {API library}
ctrl-c to exit; otherwise runs infinitely

- TIMESTAMP: 2020-06-28 06:00:10 UTC
- TOKEN STATUS: EXPIRED
- TOKEN STATUS: REFRESHED
- TOKEN STATUS: CURRENT
- TOKEN EXPIRY: 2020-06-28 07:00:11 UTC
- TOKEN: 1.36b85549******************************
- REPO OBJECT: initrode/y2k

- TIMESTAMP: 2020-06-28 06:15:11 UTC
- TOKEN STATUS: CURRENT
- TOKEN EXPIRY: 2020-06-28 07:00:11 UTC
- TOKEN: 1.36b85549******************************
- REPO OBJECT: initrode/y2k

- TIMESTAMP: 2020-06-28 06:30:11 UTC
- TOKEN STATUS: CURRENT
- TOKEN EXPIRY: 2020-06-28 07:00:11 UTC
- TOKEN: 1.36b85549******************************
- REPO OBJECT: initrode/y2k

- TIMESTAMP: 2020-06-28 06:45:12 UTC
- TOKEN STATUS: CURRENT
- TOKEN EXPIRY: 2020-06-28 07:00:11 UTC
- TOKEN: 1.36b85549******************************
- REPO OBJECT: initrode/y2k

- TIMESTAMP: 2020-06-28 07:00:12 UTC
- TOKEN STATUS: EXPIRED
- TOKEN STATUS: REFRESHED
- TOKEN STATUS: CURRENT
- TOKEN EXPIRY: 2020-06-28 08:00:12 UTC
- TOKEN: 1.d16c71c8******************************
- REPO OBJECT: initrode/y2k

- TIMESTAMP: 2020-06-28 07:15:13 UTC
- TOKEN STATUS: CURRENT
- TOKEN EXPIRY: 2020-06-28 08:00:12 UTC
- TOKEN: 1.d16c71c8******************************
- REPO OBJECT: initrode/y2k

```
Initialization immediatley recognizes that no token exists on startup and automatically refreshes itself. After an hour and exactly one second after the original token expired, the token gets refreshed automatically and the API can continue to make requests with the proper authentication/token always ensured.

## Simple Auth Class ({language-package}/autorefresh)

Using the APIenables users to keep tokens fresh with trivial amounts of code.

This example provides two classes:

- Authentication for API requests
- Requests to API endpoints wrapped in `auth `

`GitHubAPI` inherits from `GitHubAPIAuth`. Using some syntactic sugar and the `auth` method, all requests to the API get authenticated and the length between requests is handled gracefully. This isn’t much different from calling `auth` each time but `auth | do` implies requests to the API need authentication.

The sizeable difference here between a PAT and GitHub App is fairly simple. A single time check determinse if the token is still valid and if not, create a fresh instance of the client which gets a new access token. 

#### **Required class initialization parameters**
```ruby
@meta['pem'], @meta['app_id'], @meta['installation_id']
```

### Example Ruby Auth Class
```Ruby
# frozen_string_literal: true

require 'jwt'
require 'octokit'
require 'time'

# API Authentication
class GitHubAPIAuth
  def initialize(meta)
    @meta = meta
    @private_key = OpenSSL::PKey::RSA.new(File.read(@meta['pem']))
  end

  def auth
    github_client if !@auth || Integer(Time.now + 60) >= Integer(@expires_at) ensure yield
  end

  def token
    payload = { iat: Integer(Time.now), exp: Integer(Time.now + 1), iss: @meta['app_id'] }
    Octokit::Client.new(bearer_token: JWT.encode(payload, @private_key, 'RS256'))
                   .create_installation_access_token(@meta['installation_id'])
  end

  def github_client
    @auth = token
    @expires_at = @auth[:expires_at]
    @gh = Octokit::Client.new access_token: @auth[:token]
  end
end

# API Requests
class GitHubAPI < GitHubAPIAuth
  def org_repos
    auth do
      return @gh.org_repos(@meta['name'], per_page: 1, type: 'private')
    end
  end
end
```
### Octokit.rb NOTES
The `bearer` token is only valid for a single second in the example class. We assume when using a chained method that the subsequent requests will happen in the one second time frame, however that may not be a safe assumption and doesn’t account for things like latency. While I didn't experience any issues, you may want to bump this value. Last, if the token is within 60 seconds of expiring we can renew it, this mitigates edge cases where the token may be valid at the time of the check but not at the time of the API call. This would be rare, also but included for demonstration.


### Example Python Auth Class
```python
from datetime import datetime, timezone
from github import Github, GithubIntegration
import jwt


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
        return self.org.get_repos(type="private")a
```

### PyGitHub NOTES:
Unlike Octokit.rb, the default JWT token has a life span of 60 seconds.

## Recap
- Short lived JWT tokens (1 second)
- Short lived API tokens (3600 seconds)
- Inexpensive time validation for tokens
- Simple inheritance model for authorization against API endpoints
- Easier to revoke access in the event of a security incident
- No service account required
- No identity (idP) or ADFS/LDAP integration required
- PoLP

