[◀ Documentation Home](README.md)

# Kuro Configuration

The first time you boot Kuro, it will automatically generate a default configuration file named `kuro.conf` in the EFI System Partition (ESP).

The configuration file contains essential parameters for the Operating System, like its path, the logger file's location, and more. Here's an example configuration file:

```ini
# Default config file for KURO
# Please make sure the directory exists!
kernel_path=\shiro.kernel
logger_path=\kuro\logs
clear_screen=true
```

- **`kernel_path`**: Specifies the path to the kernel file (default: `\shiro.kernel`).

- **`logger_path`**: Specifies the path to the logger file (default: `\kuro\logs`).

- **`clear_screen`**: Specifies whether to clear the screen on boot (default: `true`).

> [!IMPORTANT]
> Please note that the path parameters are UEFI-Formatted, so backslashes (`\`) are used instead of forward slashes (`/`).
