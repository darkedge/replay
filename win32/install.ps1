# vcvarsall
pushd "$env:VS140COMNTOOLS..\..\VC"
cmd /c "vcvarsall.bat amd64 & set" |
    foreach {
        if ($_ -match "=") {
            $v = $_.split("="); Set-Item -Force -Path "ENV:\$($v[0])" -Value "$($v[1])"
        }
    }
popd

# This saves typing
$shell = New-Object -com Shell.Application
$webClient = New-Object Net.WebClient

# -aoa: Overwrite all without prompt
# -r: Recursive
# TODO: Make this specific per download
$7zargs = " -aoa -r *.cpp *.hpp *.c *.h *.inl *.ttf *.lua *.dasc *.bat *.lua *.dll *.lib"

# Get working directory
$workingDir = $((Split-Path -Path $MyInvocation.MyCommand.Definition -Parent) + "\")

# Output folder
$externalDir = $($workingDir + "..\external")

# Convenience functions
function Execute($cmd) {
    Write-Host $cmd
    Invoke-Expression $cmd
}

function DownloadFile($address) {
	$file = Split-Path $address -Leaf
	$dest = $($workingDir + $file)
	if ((Test-Path $dest)) {
		Write-Host $("Found " + $file)
		return
	} else {
		Write-Host $("Downloading " + $file + "...")
		$webClient.DownloadFile($address, $dest)
	}
}

function DownloadAndExtract($address) {
	$file = Split-Path $address -Leaf
	$dest = $($workingDir + $file)
	if ((Test-Path $dest)) {
		Write-Host $("Found " + $file)
	} else {
		Write-Host $("Downloading " + $address + "...")
		$webClient.DownloadFile($address, $dest)
	}

	$extension = [System.IO.Path]::GetExtension($file)
	Write-Host $("Extracting " + $file + "...")
	if ($extension -eq ".zip") {
		Execute($($workingDir + "7za.exe x " + $dest + " -o" + $externalDir + $7zargs))
	} elseif ($extension -eq ".gz") { # .tar.gz
        Execute($($workingDir + "7za.exe e " + $dest + " -o$workingDir -aoa"))

        $tar = $($workingDir + [System.IO.Path]::GetFileNameWithoutExtension($file))
        Execute($($workingDir + "7za.exe x $tar -o$externalDir $7zargs"))
	} else {
		Write-Host $($file + ": Unsupported archive format!")
	}
}

DownloadFile("http://www.7-zip.org/a/7za920.zip")

Write-Host Extracting 7-Zip executable...
foreach ($item in $shell.NameSpace($($workingDir + "7za920.zip")).Items()) {
    if ($item.Name -eq "7za.exe") {
        # 16 = "Respond with "Yes to All" for any dialog box that is displayed."
        $shell.NameSpace($workingDir).CopyHere($item, 16)
    }
}

# A recent Win32 version of make
# TODO: Compile ourselves
DownloadFile("ftp://ftp.equation.com/make/64/make.exe")

DownloadAndExtract("https://github.com/ocornut/imgui/archive/v1.49.zip")
DownloadAndExtract("http://luajit.org/download/LuaJIT-2.0.4.zip")
DownloadAndExtract("https://github.com/google/protobuf/releases/download/v2.6.1/protobuf-2.6.1.zip")
DownloadAndExtract("http://www.sfml-dev.org/files/SFML-2.4.0-windows-vc14-64-bit.zip")

# Delete downloads
function DeleteFile($file) {
    Remove-Item $($workingDir + $file) -Recurse -ErrorAction Ignore
}

DeleteFile("7za.exe")
DeleteFile("7za920.zip")
DeleteFile("LuaJIT-2.0.4.zip")
DeleteFile("protobuf-2.6.1.zip")
DeleteFile("v1.49.zip")
DeleteFile("SFML-2.4.0-windows-vc14-64-bit.zip")

# Build libraries
# TODO: Do not put the .lib files in the win32 folder
$IMGUI = "$externalDir\imgui-1.49"
$LUA = "$externalDir\LuaJIT-2.0.4\src"
$SFML = "$externalDir\SFML-2.4.0"

$BUILD = $($workingDir + "build")

$slBasicCompile = "/nologo /O2 /Gm- /MDd /GR- /EHs-c- /fp:fast /fp:except- /Oi"

# ImGui
New-Item "$BUILD\imgui" -type directory -force
$imgui_srcs = "imgui.cpp", "imgui_demo.cpp", "imgui_draw.cpp", "examples/directx11_example/imgui_impl_dx11.cpp"
$imgui_srcs = foreach($src in $imgui_srcs) { "$IMGUI\$src" }
Execute("cl /c $slBasicCompile /Fo$BUILD/imgui/ /I$IMGUI $imgui_srcs")
Execute("lib /nologo /out:imgui.lib $BUILD/imgui/*.obj")

# LuaJIT
pushd $LUA
cmd.exe /c msvcbuild.bat
popd
Copy-Item $LUA\lua51.lib $workingDir
Copy-Item $LUA\lua51.dll $workingDir
Copy-Item $LUA\luajit.exe $workingDir
New-Item $workingDir\lua\jit -type directory -force
Copy-Item $LUA\jit\* $workingDir\lua\jit
Copy-Item $LUA\jit\* $workingDir\lua\jit

# SFML (TODO: Debug vs Release)
Copy-Item $SFML\bin\openal32.dll $workingDir
Copy-Item $SFML\bin\sfml-audio-d-2.dll $workingDir
Copy-Item $SFML\bin\sfml-graphics-d-2.dll $workingDir
Copy-Item $SFML\bin\sfml-network-d-2.dll $workingDir
Copy-Item $SFML\bin\sfml-system-d-2.dll $workingDir
Copy-Item $SFML\bin\sfml-window-d-2.dll $workingDir
