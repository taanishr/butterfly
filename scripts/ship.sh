#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

ENABLE_DEBUG_UI=0
ENABLE_PROFILE=0
FRAME_WIDTH=740.0
FRAME_HEIGHT=760.0

usage() {
    cat <<EOF
Usage: scripts/ship.sh [--debug-ui] [--profile] [--frame-width N] [--frame-height N] [build|run|buildrun|compare|debug|extensions|xcode|test]

Options:
  --debug-ui  Enable the compile-time debug inspector UI
  --profile   Build with optimization, debug symbols, and frame pointers
  --frame-width N   Initial GUI content width (default: 512.0)
  --frame-height N  Initial GUI content height (default: 512.0)

Commands:
  build       Configure and build the application
  run         Run the existing application build
  buildrun    Configure, build, and run the application
  compare     Build and run the GUI with its synchronized browser reference
  debug       Configure and build, then run under lldb
  extensions  Build only the Swift AppKit/MTK extension libraries
  xcode       Compatibility alias for build
  test        Configure, build, and run the tests
EOF
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --debug-ui)
            ENABLE_DEBUG_UI=1
            shift
            ;;
        --profile)
            ENABLE_PROFILE=1
            shift
            ;;
        --frame-width)
            [[ $# -ge 2 ]] || usage
            FRAME_WIDTH="$2"
            shift 2
            ;;
        --frame-height)
            [[ $# -ge 2 ]] || usage
            FRAME_HEIGHT="$2"
            shift 2
            ;;
        --)
            shift
            break
            ;;
        -*)
            usage
            ;;
        *)
            break
            ;;
    esac
done

[[ $# -eq 1 ]] || usage
COMMAND="$1"

frame_number='^[0-9]+([.][0-9]+)?$'
[[ $FRAME_WIDTH =~ $frame_number && $FRAME_HEIGHT =~ $frame_number ]] || {
    echo "Frame width and height must be positive numbers"
    exit 1
}

if [[ $ENABLE_PROFILE -eq 1 && $ENABLE_DEBUG_UI -eq 1 ]]; then
    PRESET=profile-inspector
elif [[ $ENABLE_PROFILE -eq 1 || $COMMAND == compare ]]; then
    PRESET=profile
elif [[ $ENABLE_DEBUG_UI -eq 1 ]]; then
    PRESET=inspector
else
    PRESET=debug
fi

configure() {
    cmake --preset "$PRESET" -S "$ROOT_DIR" \
        -DGUI_FRAME_WIDTH="$FRAME_WIDTH" \
        -DGUI_FRAME_HEIGHT="$FRAME_HEIGHT"
}

build() {
    configure
    cmake --build --preset "$PRESET"
}

binary="$ROOT_DIR/build/$PRESET/gui"

case "$COMMAND" in
    build|xcode)
        build
        ;;
    run)
        [[ -x "$binary" ]] || {
            echo "Missing executable: $binary"
            echo "Run: scripts/ship.sh build"
            exit 1
        }
        exec "$binary"
        ;;
    buildrun)
        build
        exec "$binary"
        ;;
    compare)
        build

        compare_url="http://localhost:3000/compare"
        web_dir="$ROOT_DIR/tests/layout/web"
        web_pid=""

        cleanup_compare() {
            if [[ -n "$web_pid" ]]; then
                kill "$web_pid" 2>/dev/null || true
                wait "$web_pid" 2>/dev/null || true
            fi
        }
        trap cleanup_compare EXIT INT TERM

        if ! curl --silent --fail "$compare_url" >/dev/null 2>&1; then
            npm --prefix "$web_dir" run dev >"$ROOT_DIR/build/$PRESET/layout-web.log" 2>&1 &
            web_pid=$!

            for _ in {1..100}; do
                if curl --silent --fail "$compare_url" >/dev/null 2>&1; then
                    break
                fi
                if ! kill -0 "$web_pid" 2>/dev/null; then
                    echo "Browser reference server failed. See build/$PRESET/layout-web.log"
                    exit 1
                fi
                sleep 0.1
            done

            if ! curl --silent --fail "$compare_url" >/dev/null 2>&1; then
                echo "Timed out waiting for $compare_url"
                exit 1
            fi
        fi

        open "$compare_url"
        "$binary"
        ;;
    debug)
        build
        exec lldb "$binary" -o run
        ;;
    extensions)
        configure
        cmake --build --preset "$PRESET" --target gui_apple_extensions
        ;;
    test)
        configure
        cmake --build --preset "$PRESET" --target gui_bidi_test
        ctest --preset "$PRESET"
        ;;
    *)
        usage
        ;;
esac
