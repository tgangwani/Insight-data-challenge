# Insight-data-challenge
Insight Data Challenge 2016

1. Code Structure
-The main executable is a python3 script (parseTweets.py). It is used to parse and clean the
tweets. The tweets are then passed to a C++ module (orm) which builds the hashtag
graph and returns the average vertex degree back to python for each input tweet. 
-C++ and Python are linked using boost::python. The C++ source is compiled to a
shared library which is then included in python as a module. 
-Boost is also used to implement the hashtag graph. 

2. Dependencies
-The code was tested with python3. The makefile needs to have the relevant paths
in $include and $lib
-The code was tested with boost-version-1.59. The makefile needs to have the
relevant paths in $include and $lib

