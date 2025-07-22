cask "fuzzymac" do
  version "0.0.1"
  sha256 "dbde002ecc7ca231d298d481bede9148ccf9e0e3802d4c71b2cb63f35953eb51"

  url "https://github.com/MohamadCS/FuzzyMac/releases/download/v0.0.1-alpha/FuzzyMac.zip"
  name "FuzzyMac"
  desc "GUI fuzzy finder for macOS"
  homepage "https://github.com/MohamadCS/FuzzyMac"
  depends_on formula: "qt@6"

  app "FuzzyMac.app"

end
