if not exist %QtBaseDir% (
	curl -kLO https://github.com/summershrimp/obs-auto-subtitle/releases/download/0.2.2/QT_5.15.2.7z -f --retry 5 -z QT_5.15.2.7z
    7z x QT_5.15.2.7z -o%QtBaseDir%
) else (
	echo "Qt is already installed. Download skipped."
)

dir %QtBaseDir%
