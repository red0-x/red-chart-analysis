
# RedChart - The grapical charting tool, all in one for your needs
- Written in c++ with yahoo finance api implementation and simple graphing abilities.
- currently [WIP] @ [red0xx.top](https://red0xx.top/?r=redchartcpp)

>### The RoadMap
>- Add indicator functionality
>- UI Revamp with nice panelling and red coloring
>- Graphing improvements, sizing changes and better visibility
>- Add Custom candle colors and different candle types
>- EMAs, SDs and other built in indicators
>- Webhook functionality

#### 5/3/2026-5.16 

> ### Major update: Added Gui fixes and Yahoo finance fetching is now available.
- Fetch ANY chart ticker supported by yahoo finance's api and load it onto the chart

- Chart readability bugs sqaushed!

> ### Config viewing revamp,
- can now edit config within the gui and pick charts to read with ease!

<details>
<summary><b>Older updates</b></summary>

#### 8/14/2025
> ### Added Gui + Readable Chart 

- Loads Config and handles chart data.

- Doesn't really work well with large sets of data, I will be fixing that soon.

- The candles coloring can be buggy and I'm also fixing that.

- Should be more updates to come

#### 7/28/2025
> ### Added Menu

- Prints Menu 

- Handles User input

> ### Modified Parsing Output

- Looks a little different + with line counter

#### 7/24/2025
> ### Added Config Handling 

- Reads .cfg file

- Loads values from cfg

- Likely gives a segmentation fault

> ### Added CSV Handling 

- Reads .csv file

- Loads values from csv and stores them into a Chart hashmap.

- Likely gives a segmentation fault

</details>
  

## Build with 
```zsh
red0-x@red:~$ git clone https://github.com/red0-x/cppRedChart.git && cd cppRedChart/redchart/

red0-x@red:~$ make

red0-x@red:~$ ./build/redchart
```
