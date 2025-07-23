cask "fuzzymac" do
  version "0.0.2"
  sha256 "f6df8b2ef922d921206160a43377807701062b8ffbb19dbf31c463e92b1a5b75"

  url "https://github.com/MohamadCS/FuzzyMac/releases/download/v0.0.2-alpha/FuzzyMac.zip"
  name "FuzzyMac"
  desc "GUI fuzzy finder for macOS"
  homepage "https://github.com/MohamadCS/FuzzyMac"
  depends_on formula: "qt@6"

  app "FuzzyMac.app"

end
