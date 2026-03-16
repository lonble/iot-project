# RSSI-Based Environment Classification

This project focuses on **environment classification using RSSI (Received Signal Strength Indicator) data** collected from IoT sensors. The main objective is to analyze RSSI patterns from multiple sensors and determine whether different physical environments can be distinguished through machine learning and signal analysis.

## Project Overview

Wireless signals behave differently in different environments due to:
- obstacles and walls,
- multipath fading,
- attenuation,
- distance variations,
- environmental interference.

In this project, RSSI data was collected using multiple sensors and processed to build a classification pipeline. The workflow includes:
- raw data collection,
- preprocessing and cleaning,
- feature extraction,
- model training,
- performance evaluation,
- visualization of results.

## Objectives

The main goals of this project are:
1. Collect RSSI data from different environments.
2. Preprocess and organize the sensor data.
3. Extract meaningful statistical or temporal features.
4. Train machine learning models for classification.
5. Evaluate model performance using standard metrics.
6. Analyze whether RSSI is reliable enough for environment recognition.

## Dataset

The dataset consists of RSSI readings obtained from IoT sensor nodes. Different files correspond to measurements collected under different environmental conditions and from different sensors.

### Data Notes
- RSSI values are measured in dBm.
- Multiple sensors were used.
- Data may include timestamps, addresses, RSSI values, and LQI values.
- The dataset was later transformed into a machine-learning-ready format.

## Methodology

### 1. Data Collection
RSSI signals were collected using embedded wireless sensor nodes. One sensor transmitted signals while other sensors recorded RSSI and related communication indicators.

### 2. Data Preprocessing
Preprocessing involved:
- removing invalid or noisy rows,
- formatting sensor logs,
- combining multiple files,
- assigning labels for each environment,
- preparing the final dataset for model training.


