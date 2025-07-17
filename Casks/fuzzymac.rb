cask "fuzzymac" do
  version "0.0.1"
  sha256 "566e99cf1f5e703f9485d2ec133c147666f6024bbcf0ea1b6ddd67f396c7972a"

  url "https://github.com/MohamadCS/FuzzyMac/releases/download/v0.0.1-alpha/FuzzyMac.zip"
  name "FuzzyMac"
  desc "GUI fuzzy finder for macOS"
  homepage "https://github.com/MohamadCS/FuzzyMac"
  depends_on formula: "qt@6"

  app "FuzzyMac.app"

end
