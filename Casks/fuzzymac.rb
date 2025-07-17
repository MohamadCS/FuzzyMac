cask "fuzzymac" do
  version "0.0.1"
  sha256 "9915565c07654997fd146086c2ef12d3c710709f4e6e13eee6be0467bab68247"

  url "https://github.com/MohamadCS/FuzzyMac/releases/download/v0.0.1-alpha/FuzzyMac.zip"
  name "FuzzyMac"
  desc "GUI fuzzy finder for macOS"
  homepage "https://github.com/MohamadCS/FuzzyMac"
  depends_on formula: "qt@6"

  app "FuzzyMac.app"

end
