# fuzzymac.rb
class Fuzzymac < Formula
  desc "GUI fuzzy finder for macOS"
  homepage "https://github.com/MohamadCS/FuzzyMac"
  url "https://github.com/MohamadCS/FuzzyMac/releases/download/v0.0.1-alpha/fuzzymac.tar.gz"
  sha256 "3b2895d30da8708d734713e26ab90a7b887ea9a92369651401ad5958b1619759"  # Replace with real checksum
  license "MIT"

  def install
    prefix.install "FuzzyMac/FuzzyMac.app"
    system "ln", "-sfn", "#{prefix}/FuzzyMac.app", "/Applications/FuzzyMac.app"
  end

  def caveats
    <<~EOS
    EOS
  end

  def uninstall
  end
end
