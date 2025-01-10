# Scheduler CHOP Plugin for TouchDesigner

**Scheduler CHOP** is a custom C++ plugin designed to bring powerful time-based scheduling capabilities to your TouchDesigner projects. With this plugin, you can trigger events or control behaviors based on global or day-specific schedules, making it perfect for interactive installations, performances, and time-sensitive tasks.

---

## Features

### ✅ Global Scheduling Mode
- Configure a single time range that applies across all days.
- Automatically disables day-specific schedules to prevent conflicts.

### ✅ Day-Specific Scheduling
- Set unique schedules for each day of the week.
- Define start and end times down to the second for precise control.

### ✅ Trigger Logic
- Outputs a `Trigger` channel that indicates whether the schedule is active (`1`) or inactive (`0`).
- Includes a user-defined interval to prevent multiple rapid triggers.

### ✅ Dynamic UI
- Automatically grays out day-specific parameters when Global Mode is enabled.
- Makes the interface intuitive and avoids misconfigurations.

### ✅ High Performance
- Runs efficiently at 60Hz for real-time applications.

---

## How to Install and Use

1. **Download the Plugin**:
   - Clone or download this repository.

2. **Locate the DLL File**:
   - Inside the repository folder, find the file named `SchedulerCHOP.dll` in the `DLL` folder.

3. **Place the DLL in the Plugins Folder**:
   - Move the `SchedulerCHOP.dll` file to the following directory:
     ```
     C:\Users\YourUsername\Documents\Derivative\Plugins
     ```
     Replace `YourUsername` with your Windows username.

4. **Open TouchDesigner**:
   - Launch TouchDesigner.
   - Open the node browser, and you will find the `SchedulerCHOP` plugin under the **Custom** tab.

5. **Add the SchedulerCHOP Node**:
   - Drag the SchedulerCHOP node into your network.
   - Configure the parameters to set up your desired schedule.

---

## How It Works

### Parameters

#### Global Mode
- **Global Mode Toggle**: Enables or disables global scheduling.
- **Global Start/End Time**: Set the start and end times for the global schedule.
- **Interval**: Define the minimum interval (in seconds) between triggers.

#### Day-Specific Mode
Each day of the week has its own parameters:
- **Enable**: Activates scheduling for that specific day.
- **Start/End Time**: Define the active time range for that day.

### Trigger Output
The plugin generates a `Trigger` channel that:
- Outputs `1` when the current time falls within the active schedule.
- Outputs `0` otherwise.

---

## Example Usage

### Global Schedule
1. Enable **Global Mode**.
2. Set the start and end times for the global schedule.
3. Use the `Trigger` channel to control nodes based on the active schedule.

### Day-Specific Schedule
1. Disable **Global Mode**.
2. Enable specific days and configure their time ranges.
3. Use the `Trigger` channel to respond to day-specific schedules.

---

## Use Cases

- **Interactive Installations**: Schedule when visuals or audio are active.
- **Performances**: Automate effects at specific times.
- **Event Management**: Manage daily event schedules efficiently.

---

## Author

**Data_C0re_**  
📧 [datacoredrive1@gmail.com](mailto:datacoredrive1@gmail.com)  
📷 [Instagram: @data_c0re_](https://www.instagram.com/data_c0re_/)

Feel free to contribute to this repository or suggest improvements!
