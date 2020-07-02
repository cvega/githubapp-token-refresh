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
