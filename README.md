# Simu5G-MobiEdgeSim

## Overview

Simu5G-MobiEdgeSim is developed based on Simu5G v1.2.2, an OMNeT++ library designed for the end-to-end performance evaluation of 5G networks. It maintains compliance with the original project's LGPL v3 license.

## Main Modifications and Enhancements

### Mobile MEC Hosts

- Implemented mechanisms for MEC hosts to move dynamically within simulation environments, enabling more realistic modeling of mobile edge scenarios.

### Extended MEC Host Information Collection

- Enhanced MEC host modules to collect and report a wider range of metrics, facilitating detailed performance analysis.

### Location-aware Parameters for UEs and MEC Hosts

- Integrated location-related parameters into User Equipment (UE) and MEC host modules, supporting spatially-aware simulation scenarios.

### Improved MEC Host Selection Policy

- Updated the MEC host selection mechanism by defining clear interfaces and integrating with the external placement policy implementation available at [MobiEdgeSim/PlacementPolicy](https://github.com/MobiEdgeSim/PlacementPolicy). This facilitates flexible and optimized task placement.

### Cellular Module Integration

- Added a cellular module to MEC hosts to connect the entire network, maintaining the MEC architecture's integrity and enhancing network connectivity.

### Wi-Fi-based Task Placement Simulation

- Developing a task placement simulation framework based on Wi-Fi communications. For more details, see the repository [MobiEdgeSim_WIFI](https://github.com/MobiEdgeSim/MobiEdgeSim_WIFI).

## Required Software Versions

- **OMNeT++:** 6.0.3
- **SUMO:** 1.11.0
- **INET:** 4.5
- **Veins:** 5.2

## License and Original Work

Simu5G-MobiEdgeSim remains under the GNU Lesser General Public License (LGPL) v3. The original LICENSE file and the original README are preserved within this repository as `LICENSE` and `README.original.md`, respectively.

**Original authors:** G. Nardini, D. Sabella, G. Stea, P. Thakkar, A. Virdis (University of Pisa, Intel)

## Contact

For questions or further information, please contact the repository maintainer at [zhangt@tcd.ie](mailto:zhangt@tcd.ie). Emails will be replied to periodically.

