$makeNsis = 'C:\Program Files\NSIS\makensis.exe'
$myDir = (get-item $($myinvocation.InvocationName)).DirectoryName
$rootDevDir = (get-item $mydir).Parent.Parent.FullName
$nsisScript = "$myDir/setup.nsi"
$zipInstallFile = "$rootDevDir/PortableKFLog.zip"
$exeInstallFile = "$rootDevDir/KflogSetup.exe"

"hier: $rootDevDir"

$QtFiles = "QtCore4.dll",
           "QtGui4.dll",
           "QtNetwork4.dll",
           "QtOpenGL4.dll",
           "QtXml4.dll"

$MinGwFiles = "libgcc_s_dw2-1.dll",
              "..\mingw32\opt\bin\libwinpthread-1.dll",
              "libstdc++-6.dll"
$KflogFiles = "kflog\Release\kflog.exe"

if ($env:QT_DIR -eq $NULL) 
{
    $QtDirectory = "C:\Qt\4.8.6"
}
else
{
    $QtDirectory = $env:QT_DIR
}

# MinGw directory
$gccObject = (get-item (Get-Command gcc).Definition)
$mingwDirectory = $gccObject.DirectoryName

if ($args.Length -eq 0)
{
    $MakeDirectory = "C:/Users/peter/dev/KFLog/build-windows_setup-Desktop-Release"
}
else
{
    $MakeDirectory = $args
}

$kflogMakeDirectory = "$MakeDirectory/kflog/release"
# C:\Users\peter\dev\build-kflog-Desktop-Release\kflog\release

# create working directory if not existing
$workDirectory = "$makeDirectory/tmp"
if (-not (test-path $workDirectory))
{
    New-Item -ItemType Directory $workDirectory
}

Remove-Item -Recurse $workDirectory/*

# copy the files to the working directory
$QtFiles | %{Copy-Item -Verbose $QtDirectory/bin/$_ $workDirectory}
$MinGwFiles | %{Copy-Item -Verbose $MinGwDirectory/$_ $workDirectory}
$KflogFiles | %{Copy-Item -Verbose $MakeDirectory/$_ $workDirectory}

$zipfile = "$MakeDirectory/../../PortableKFLog.zip" 
if ((test-path $zipfile))
{
    Remove-Item $zipfile
}

# Load the compression namespace 
# and yes, I know this usage is obsolete - but it works. 
# Ignore the output 
[System.Reflection.Assembly]::LoadWithPartialName('System.IO.Compression.FileSystem') | Out-Null  
 
# Create a type accellerator for the zipfile class 
[System.Type] $TypeAcceleratorsType=[System.Management.Automation.PSObject].Assembly.GetType('System.Management.Automation.TypeAccelerators',$True,$True) 
$TypeAcceleratorsType::Add('Zipfile','System.IO.Compression.Zipfile') 

$zipfile
$workDirectory
     
# Now create the Zip file 
Try { 
  [Zipfile]::CreateFromDirectory( $workDirectory, $zipfile) 
  "Zip file created:";ls $zipfile} 
Catch { 
  "Zip File NOT created" 
  $Error[0]} 

if ((Test-Path $zipInstallFile))
{
    Remove-Item $zipInstallFile
}
Move-Item -Verbose $zipfile $zipInstallFile

  &"$makeNSIS" "/DMingwBinPath=$MinGwDirectory" "/DQtBinPath=$QtDirectory/bin" "/DExeSourcePath=$kflogMakeDirectory" "/NOCONFIG" "$nsisScript" 

if ((Test-Path $exeInstallFile))
{
    Remove-Item $exeInstallFile
}
Move-Item -Verbose "$myDir/KflogSetup.exe" $exeInstallFile
