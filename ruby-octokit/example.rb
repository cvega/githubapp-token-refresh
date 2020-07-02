#!/usr/bin/env ruby
# frozen_string_literal: true

# EXAMPLE AUTOMATED TOKEN REFRESH USING GITHUB APP AND OCTOKIT.RB
#
# This example is designed to highlight automated token rotation by validating
# that the token has not expired. If the token has expired, we reinitialize
# the octokit client. This allows us to use short-lived keys and reduce the
# overall amount of API requests.
#
# NOTE:
# This requires a Github App with read-only access to the repo data. For
# example purposes, we only query for a single result/repo.

require 'jwt'
require 'octokit'
require 'optparse'
require 'time'
require 'yaml'

ENV['OCTOKIT_SILENT'] = 'true'

# parse CLI options
options = { config: '.config.yml', sleep: 300, expires_after: 3600 }
OptionParser.new do |opts|
  opts.banner = "Usage: #{__FILE__} [options]"
  opts.on('-c', '--config PATH', String, '$PATH to config file') do |v|
    if File.file?(v)
      options[:config] = v
    else
      puts "Error: invalid config file path\n\n"
      puts opts
      exit 1
    end
  end
  opts.on('-s', '--sleep SECONDS', Integer, 'Number of seconds between requests') do |v|
    options[:sleep] = v
  end
  opts.on('-e', '--expires-after SECONDS', Integer, 'Number of seconds before token expiration') do |v|
    options[:expires_after] = v if v.positive? && v <= 3600
  end
  opts.on_tail('-h', '--help', 'show this message') do
    puts opts
    exit
  end
  opts.parse!
end

# GitHubAPIAuth provides an authenticated layer to GithubAPI
class GitHubAPIAuth
  def initialize(meta, opts)
    @meta = meta
    @opts = opts
    @private_key = OpenSSL::PKey::RSA.new(File.read(@meta['pem']))
  end

  # NOTE:
  # Determines if the token is expired using the expires_at attribute returned
  # from the octokit client. This prevents superfluous API requests. Tokens can
  # be forced to expire using the --expires-after parameter for this example.
  # By default, they are valid for 1 hour.
  def auth
    if !@auth || Integer(Time.now + (3600 - @opts[:expires_after])) >= Integer(@expires_at)
      puts '- TOKEN STATUS: EXPIRED'
      github_client
      puts '- TOKEN STATUS: REFRESHED'
    end
    puts '- TOKEN STATUS: CURRENT'
    puts "- TOKEN EXPIRY: #{@expires_at}"
    puts "- TOKEN: #{@auth[:token][1..10]}" + '*' * 30
    yield
  end

  # request access token using bearer auth (jwt token)
  def token
    payload = { iat: Integer(Time.now), exp: Integer(Time.now + 1), iss: @meta['app_id'] }
    Octokit::Client.new(bearer_token: JWT.encode(payload, @private_key, 'RS256'))
                   .create_installation_access_token(@meta['installation_id'])
  end

  # create github api client
  def github_client
    @auth = token
    @expires_at = @auth[:expires_at]
    @gh = Octokit::Client.new access_token: @auth[:token]
  end
end

# create API requests
#
# NOTE:
# GitHubAPIAuth provides the auth attribute to keep tokens fresh
class GitHubAPI < GitHubAPIAuth
  # repos for each org
  def org_repos
    # wrap authenticated requests with `auth` to ensure token is not expired
    auth do
      @gh.org_repos(@meta['name'], per_page: 1, type: 'public')
    end
  end
end

# load yaml config
config = YAML.safe_load(File.read(options[:config]))
# main loop, iterate config file
config['orgs'].each do |meta|
  ghapi = GitHubAPI.new meta, options
  # run reports
  puts 'EXAMPLE AUTOMATED TOKEN REFRESH USING GITHUB APP AND OCTOKIT.RB'
  puts 'ctrl-c to exit; otherwise runs infinitely'
  puts
  loop do
    puts "- TIMESTAMP: #{Time.now.utc.strftime('%Y-%m-%d %H:%M:%S %Z')}"
    puts "- REPO OBJECT: #{ghapi.org_repos[0].full_name}"
    puts
    sleep options[:sleep]
  end
end
