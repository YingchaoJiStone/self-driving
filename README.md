# 2024-group-19

![Pipeline status](https://git.chalmers.se/courses/dit638/students/2024-group-19/badges/main/pipeline.svg)

## Getting started

### Dependencies

The following tools are required to run the project:
```cmake, make, git```

To install them on an Ubuntu 22.04 system, run the following commands:
```sh
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install cmake build-essential git
```

### Clone the repo

To get started, checkout the repository with the following command:
```sh
git clone https://git.chalmers.se/courses/dit638/students/2024-group-19.git
```

### Building the project

To build the project, cd into the repository, and run the following commands:
```sh
mkdir build
cd build
cmake ..
make
```

## Team rules on adding new features and fixing unexpected behaviour
*Parts of this section where automatically generated using Mistral 7B 0.2 run with Ollama 0.1.29*

### Adding new features

To implement a new feature, follow these steps:
1. Open a new issue in GitLab, detailing the desired feature.
2. Initiate a new branch specifically for this feature.
3. Develop and integrate the new feature within the branch.
4. Submit a merge request to merge the branch into the main project.
5. Designate a team member to review the merge request.
6. After approval, the merge request can be merged with the main branch.

### Fixing unexpected behaviour

For addressing unexpected behavior in the project, follow these steps:
1. Report the issue by creating a new entry on GitLab, including a detailed description of the unwanted behavior.
2. Initialize a new branch for the fix.
3. Find and implement the solution to rectify the issue within the branch.
4. Submit a merge request for merging the branch back into the main project.
5. Allocate a team member to review the merge request.
6. Upon completion of the review, the merge request can be merged with the main branch.


## Commit message structures

The following format will be used for all commit messages:
```#12 Add feature to turn car left```

Commit messages should begin with the issue number the commit is addressing. The commit message should be simple, descriptive and to the point, it should be written using the imperative, present tense (i.e. "add changes" instead of "added changes"). 

Optionally commit messages can include a longer description in the body when necessary.
