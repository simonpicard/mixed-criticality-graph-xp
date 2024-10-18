# mixed-criticality-graph-xp

## Pre-requisites

(optional) If you'd like to leverage the automation provided by docker, you
need to [install it](https://docs.docker.com/engine/install/).

## Build and run evaluation with docker

Clone the repo and run the helper script:

```bash
git clone git@github.com:simonpicard/mixed-criticality-graph-xp.git
git submodule update --init
cd mixed-criticality-graph-xp/
./docker.sh make
```

## Cite
If you find MC Graph explorer useful for your research and applications, please cite us using this BibTeX:

```bibtex
@InProceedings{picard2024mcgraph,
  title={Exact schedulability test for sporadic mixed-criticality real-time systems using antichains and oracles},
  author={Simon Picard and Antonio Paolillo and Joël Goossens and Gilles Geeraerts},
  journal={RTNS 2024},
  year={2024}
}
```
