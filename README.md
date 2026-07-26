# Tribe

Tribe is an address book manager for the GNOME and Xfce desktop environments.

## Dependencies

Tribe depends on the following runtime libraries:

* [GLib](https://gitlab.gnome.org/GNOME/glib/) >= 2.19.7
* [GTK](https://www.gtk.org) >= 2.16.1

## Requirements & Prerequisites

To build Tribe from source, you'll need the following tools and development files:

* **CMake** >= 2.6
* **C compiler** (e.g., GCC or Clang)
* **Make**
* **pkg-config**
* **Development headers** for GLib and GTK (usually provided by `-dev` or `-devel` packages)

On Debian/Ubuntu:
```sh
sudo apt-get install cmake build-essential pkg-config libglib2.0-dev libgtk2.0-dev
```

On Fedora/RHEL:
```sh
sudo dnf install cmake gcc make pkg-config glib2-devel gtk2-devel
```

## Build & Install

This project uses [CMake](https://cmake.org/) as its build system.

### 1. Configure the Build

Create a dedicated directory for the build and generate the build files:

```sh
mkdir out && cd out
cmake -DCMAKE_BUILD_TYPE=Release ..
```

**Configuration Options:**

- `-DCMAKE_BUILD_TYPE=Release` — Builds an optimized release version (default). Use `Debug` if you need debug symbols for development or troubleshooting.
- `-DCMAKE_INSTALL_PREFIX=/usr` — Sets the installation prefix. By default, Tribe installs to `/usr/local`. Change this if you want to install to a different location (e.g., `/usr`, `/opt`, or `$HOME/.local`).

**Example: Installing to `/usr` with debug symbols:**

```sh
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr ..
```

### 2. Compile the Project

Build the project with:

```sh
make
```

### 3. Install

Install the resulting binaries and other files:

```sh
sudo make install
```

**Note:** The `sudo` command is required if you're installing to system directories like `/usr` or `/usr/local`. If you installed to a user-writable directory (e.g., `$HOME/.local`), you may omit `sudo`.

### 4. Uninstall

To remove Tribe from your system:

```sh
sudo make uninstall
```

## Contributing

We welcome contributions! To get started:

1. **Fork the repository** on GitHub
2. **Create a feature branch** for your changes
3. **Make your changes** following the existing code style
4. **Build and test** your changes using the build instructions above
5. **Submit a pull request** with a clear description of your changes

When contributing, please ensure:
- Your code follows the existing C coding style (C99 standard)
- You test your changes before submitting
- Your commit messages are clear and descriptive

## License

Tribe is released under the [GNU General Public License v2.0](COPYING). See the [COPYING](COPYING) file for the full license text.
