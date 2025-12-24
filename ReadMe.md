[WIP] @ [red0xx.top](https://red0xx.top/?r=gh)
# red0xx's chart analysis tool

>### The RoadMap
>- Read and Load Chart Data from .csv
>- Find where the most liquidity gets taken out
>- Lose money
>- Improve my C++ skills

#### 7/24/2025
> ### Added Config Handling

- Reads .cfg file

- Loads values from cfg

- Likely gives a segmentation fault

> ### Added CSV Handling 

- Reads .csv file

- Loads values from csv and stores them into a Chart hashmap.

- Likely gives a segmentation fault

#### 7/28/2025
> ### Added Menu

- Prints Menu 

- Handles User input

> ### Modified Parsing Output

- Looks a little different + with line counter

#### 8/14/2025
> ### Added Gui + Readable Chart 

- Loads Config and handles chart data.

- Doesn't really work well with large sets of data, I will be fixing that soon.

- The candles coloring can be buggy and I'm also fixing that.

- Should be more updates to come
  

## Build with 
```zsh
red0-x@red:~$ git clone https://github.com/red0-x/red-chart-analysis.git && cd red-chart-analysis/red-chart-analysis

red0-x@red:~$ make

red0-x@red:~$ ./build/redchart
```
