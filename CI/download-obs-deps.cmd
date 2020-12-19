if not exist %DepsBasePath% (
    curl -o %DepsBasePath%.zip -kLO https://cdn-fastly.obsproject.com/downloads/dependencies2019.zip -f --retry 5 -C -
    7z x %DepsBasePath%.zip -o%DepsBasePath%
) else (
    echo "OBS dependencies are already there. Download skipped."
)
