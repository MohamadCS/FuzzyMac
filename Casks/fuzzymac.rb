cask "fuzzymac" do
  version "0.0.3"
  sha256 "1da443114a6b77274c7a33f3fa237fbd044b7ad56d517900f8d5fb5f911fb149"

  url "https://github.com/MohamadCS/FuzzyMac/releases/download/v0.0.3-alpha/FuzzyMac.zip"
  name "FuzzyMac"
  desc "GUI fuzzy finder for macOS"
  homepage "https://github.com/MohamadCS/FuzzyMac"
  depends_on formula: "qt@6"

  app "FuzzyMac.app"

end
