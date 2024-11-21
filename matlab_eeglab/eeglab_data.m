% Define the path to your CSV file
csvFilePath = 'C:\Users\User\Desktop\eeg_data_emotion2.csv';

% Read the CSV file into a table
eegTable = readtable(csvFilePath);

% Extract EEG channels
eegData = eegTable{:, {'Differential1', 'Differential2', 'Differential3'}};

% Extract timestamps
timestamps = eegTable.Local_Timestamp;

% Convert timestamps to numerical format if necessary
% For simplicity, we'll assume data is uniformly sampled at 256 Hz
% and generate a time vector based on the number of samples
fs = 256; % Sampling rate in Hz
numSamples = size(eegData, 1);
duration = numSamples / fs;
timeVector = linspace(0, duration, numSamples);

% Transpose EEG data to (channels x samples)
eegData = eegData';

% Define channel names
channelNames = {'Differential1', 'Differential2', 'Differential3'};

% Create channel locations structure
% If you have specific electrode positions, you can define them here.
% For simplicity, we'll assign generic positions.
% You can skip this step or assign standard locations as needed.

% Example: Assigning standard 10-20 system locations
% Note: With only 3 differential channels, spatial plotting might be limited.

% Initialize channel locations
chanlocs = struct('labels', channelNames, 'type', repmat({'EEG'}, 1, 3));

% (Optional) Assign standard positions if known
% For example, let's assign arbitrary positions:
chanlocs(1).X = 0; chanlocs(1).Y = 0; chanlocs(1).Z = 0;
chanlocs(2).X = 1; chanlocs(2).Y = 0; chanlocs(2).Z = 0;
chanlocs(3).X = 0; chanlocs(3).Y = 1; chanlocs(3).Z = 0;

% Create EEGLAB dataset structure
EEG = eeg_emptyset; % Initialize empty EEGLAB dataset

% Assign data
EEG.data = eegData;

% Assign number of channels and samples
EEG.nbchan = size(eegData, 1);
EEG.pnts = size(eegData, 2);
EEG.trials = 1; % Continuous data
EEG.srate = fs; % Sampling rate
EEG.xmin = 0; % Start time in seconds
EEG.xmax = duration; % End time in seconds

% Assign channel names and types
EEG.chanlocs = chanlocs;
EEG.setname = 'Imported_CSV_EEG_emotion2';
EEG.filename = 'Imported_CSV_EEG_emotion2.set';
EEG.comments = 'EEG data imported from CSV file';

% Set event information if available
% For continuous data without events, this can be left empty
EEG.event = [];

% Save the dataset to a .set file
pop_saveset(EEG, 'filename', EEG.filename, 'filepath', 'C:\Users\User\Desktop\');

