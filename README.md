# Simu5G-MobiEdgeSim

## Overview

Simu5G-MobiEdgeSim extends Simu5G v1.2.2, an OMNeT++ library designed for evaluating service placement performance of 5G networks. It incorporates dynamic and realistic features to simulate both static and mobile Multi-access Edge Computing (MEC) server scenarios. This project is compliant with the original LGPL v3 license.

## Key Extensions and Enhancements

### Mobile MEC Hosts

- Integrated mobile MEC hosts that dynamically move within simulated environments using realistic mobility patterns from SUMO traffic simulations through Veins.

### Extended MEC Host Information Collection

- Expanded data collection capabilities in MEC hosts, capturing comprehensive metrics to support detailed performance analysis.

### Location-aware Parameters

- Introduced geographical parameters (latitude and longitude) for MEC hosts and user equipment (UEs), enabling location-aware service placement and simulation.

### Enhanced MEC Host Selection Policy

- Developed clear selection interfaces for MEC hosts, integrating placement policies via an external module available at [MobiEdgeSim/PlacementPolicy](https://github.com/MobiEdgeSim/PlacementPolicy).

### Cellular Module Integration

- Added a cellular communication module within MEC hosts to maintain robust connectivity across the network, preserving the overall MEC architecture.


## License and Original Work

Simu5G-MobiEdgeSim remains under the GNU Lesser General Public License (LGPL) v3. Original LICENSE and README files are preserved as `LICENSE` and `README.original.md`, respectively.

**Original authors:** G. Nardini, D. Sabella, G. Stea, P. Thakkar, A. Virdis (University of Pisa, Intel)

## Contact

For questions or further information, please contact the repository maintainer at [zhangt@tcd.ie](mailto:zhangt@tcd.ie). Emails will be replied to periodically.