$formatPath = $args[0] + "clang-format"
$changedFiles = git diff --name-only main
$validExtensions = "cpp", "h", "inl"
$exitCode = 0

$localPath = Split-Path $MyInvocation.MyCommand.Path

$changedFiles | ForEach-Object {
  $extension = (Split-Path -Path $_ -Leaf).Split(".")[-1];
  if ($validExtensions -contains $extension)
  {
    $file = $_
    $fileRelPath = $localPath+"\..\"+$_
    ForEach ($line in & "$formatPath" -output-replacements-xml $fileRelPath)
    {
      if ( $line.StartsWith("<replacement ") )
      {
        Write-Host $file needs to be formatted
        $exitCode = 1
        break
      }
    }
  }
}

exit $exitCode
