# :factory: :shower: Automatically refreshing GitHub App tokens with ruby

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