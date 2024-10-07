# PjPlots

A plotting library that is a bit OTT and uses modern C++ for fast, efficient, and type-safe code. PjPlots is designed to be self-contained with no external dependencies. It is written in C++20 and is intended to be used with CMake.

## Requirements

- C++20 compiler (tested with GCC 14.2.1)
- CMake 3.5+

## Features

- Supports 2D plots
- Supports multiple colours
- Supports multiple line styles
- Supports multiple marker styles
- Supports multiple plot types (line, scatter, bar)
- Supports multiple grid styles
- Generic N-D array/matrix types supporting both static and dynamic memory allocation


## Example

```cpp
#include "PjPlots.h"


PjPlot::Factory builder;
builder.get_appearance_options().set_background_colour(PjPlot::Colour::BLACK);
builder.get_appearance_options().set_text_colour(PjPlot::Colour::WHITE);

const auto img_static = builder.get_plot<PjPlot::LineChart, double>(arr, PjPlot::LineChart::Params(k_num_series, k_series_length), PjPlot::StaticSize2<600, 600>{});

const auto img_dynamic = builder.get_plot<PjPlot::LineChart, double>(arr, PjPlot::LineChart::Params(k_num_series, k_series_length), PjPlot::DynamicSize2(600, 600));

```

## Licence

PjPlots is licensed under the BSD 3-Clause Licence. See the [LICENCE.txt](LICENCE.txt) file for details.   
