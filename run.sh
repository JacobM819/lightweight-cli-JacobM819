#!/bin/bash

cd src && make && cd .. && cd tests && grade.sh

# chmod u+x run.sh