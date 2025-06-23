# Pratical Assigment 1 - Election Day

## Usage

```sh
cd code
```

```sh
javac -d bin -sourcepath src src/Main/Main.java
```

```sh
java -cp bin Main.Main
```

Note: Tested in `openjdk version "21.0.6"`.

## Guidelines for the implementation

### 1. Characterize State-Level Interaction
- **Define how different entities interact by modifying or accessing shared state information.**
  - **PollingStation**: Manages the state of the polling station, including the queue of voters, the validation of voter IDs, and the voting process. It interacts with `Voter`, `PollClerk`, and `ElectionSimulationGUI`.
  - **Voter**: Represents an individual voter who goes through various states such as entering the queue, waiting for validation, voting, and exiting. It interacts with `PollingStation` and `ExitPoll`.
  - **PollClerk**: Responsible for validating voter IDs and processing voters. It interacts with `PollingStation` and `Voter`.
  - **Pollster**: Conducts exit polls with voters to gather responses. It interacts with `ExitPoll`.
  - **ExitPoll**: Manages the state of exit polls, including the list of eligible voters and their responses. It interacts with `Voter` and `Pollster`.
  - **ElectionSimulationGUI**: Provides a graphical interface for the election simulation, displaying the state of the polling station and the results. It interacts with `PollingStation` and `LogRepository`.
  - **LogRepository**: Handles the logging and saving of the election results to a file. It interacts with `ElectionSimulationGUI`.

- **Identify the key state variables and their expected transformations throughout the execution.**
  - **PollingStation**:
    - `open`: Indicates whether the polling station is open or closed.
    - `waitingQueue`: Queue of voters waiting to be processed.
    - `votedIDs`: Set of voter IDs that have already voted.
    - `stationSemaphore`: Controls the capacity of the polling station.
    - `votingSemaphore`: Controls access to the voting process.
  - **Voter**:
    - `currentState`: The current state of the voter (INIT, ENTER_QUEUE, WAIT_FOR_VALIDATION, VOTE, EXIT, REBORN, DONE).
    - `validated`: Indicates whether the voter has been validated.
  - **PollClerk**:
    - `currentState`: The current state of the poll clerk (INIT, WAIT_FOR_VOTER, VALIDATE, DONE).
    - `processedVoters`: The number of voters processed by the poll clerk.
    - `currentVoter`: The voter currently being processed.
  - **Pollster**:
    - `currentState`: The current state of the pollster (INIT, WAIT, CONDUCT_POLL, DONE).
  - **ExitPoll**:
    - `active`: Indicates whether the exit poll is active.
    - `eligibleVoters`: List of voter IDs eligible for the exit poll.
  - **ElectionSimulationGUI**:
    - `scoreA`: The number of votes for candidate A.
    - `scoreB`: The number of votes for candidate B.
    - `tableModel`: The model for the table displaying the election results.
  - **LogRepository**:
    - Handles the saving of the election results to a file.

### 2. Specify the Life Cycle and Internal Properties of Each Entity
- **Outline the creation, execution, and termination stages of each participating entity.**
  - **PollingStation**:
    - **Creation**: Initialized with a capacity, GUI reference, log repository, and vote counter.
    - **Execution**: Manages the queue of voters, validates voter IDs, and handles the voting process.
    - **Termination**: Closes the station and updates the GUI.
  - **Voter**:
    - **Creation**: Initialized with a voter ID, polling station reference, and exit poll reference.
    - **Execution**: Goes through states such as INIT, ENTER_QUEUE, WAIT_FOR_VALIDATION, VOTE, EXIT, REBORN, and DONE.
    - **Termination**: Marks the voter as DONE and optionally reborns as a new voter.
  - **PollClerk**:
    - **Creation**: Initialized with a polling station reference and the maximum number of voters to process.
    - **Execution**: Validates voter IDs and processes voters.
    - **Termination**: Marks the poll clerk as DONE after processing the maximum number of voters or when the station is closed.
  - **Pollster**:
    - **Creation**: Initialized with an exit poll reference.
    - **Execution**: Conducts exit polls with voters.
    - **Termination**: Marks the pollster as DONE when the exit poll is no longer active.
  - **ExitPoll**:
    - **Creation**: Initialized with a log repository reference.
    - **Execution**: Manages the list of eligible voters and conducts exit polls.
    - **Termination**: Closes the exit poll and stops accepting new voters.
  - **ElectionSimulationGUI**:
    - **Creation**: Initialized with a log repository reference and sets up the GUI components.
    - **Execution**: Updates the GUI based on the state of the polling station and the voting process.
    - **Termination**: Saves the election results to a file and closes the GUI.
  - **LogRepository**:
    - **Creation**: No specific initialization required.
    - **Execution**: Handles the logging and saving of the election results to a file.
    - **Termination**: No specific termination required.
  - **VoteCounter**:
    - **Creation**: No specific initialization required.
    - **Execution**: Manages the counting of votes for each candidate.
    - **Termination**: No specific termination required.

- **Define internal attributes that determine the behavior and transitions of each entity.**
  - **PollingStation**:
    - `open`: Indicates whether the polling station is open or closed.
    - `waitingQueue`: Queue of voters waiting to be processed.
    - `votedIDs`: Set of voter IDs that have already voted.
    - `stationSemaphore`: Controls the capacity of the polling station.
    - `votingSemaphore`: Controls access to the voting process.
  - **Voter**:
    - `currentState`: The current state of the voter (INIT, ENTER_QUEUE, WAIT_FOR_VALIDATION, VOTE, EXIT, REBORN, DONE).
    - `validated`: Indicates whether the voter has been validated.
  - **PollClerk**:
    - `currentState`: The current state of the poll clerk (INIT, WAIT_FOR_VOTER, VALIDATE, DONE).
    - `processedVoters`: The number of voters processed by the poll clerk.
    - `currentVoter`: The voter currently being processed.
  - **Pollster**:
    - `currentState`: The current state of the pollster (INIT, WAIT, CONDUCT_POLL, DONE).
  - **ExitPoll**:
    - `active`: Indicates whether the exit poll is active.
    - `eligibleVoters`: List of voter IDs eligible for the exit poll.
  - **ElectionSimulationGUI**:
    - `scoreA`: The number of votes for candidate A.
    - `scoreB`: The number of votes for candidate B.
    - `tableModel`: The model for the table displaying the election results.
  - **LogRepository**:
    - Handles the saving of the election results to a file.
  - **VoteCounter**:
    - `votesA`: The number of votes for candidate A.
    - `votesB`: The number of votes for candidate B.

### 3. Define Information Sharing Regions
- **Specify the internal data structure used for storing shared information.**
  - **PollingStation**:
    - `waitingQueue`: A `Queue<Voter>` to store voters waiting to be processed.
    - `votedIDs`: A `Set<Integer>` to store IDs of voters who have already voted.
    - `stationSemaphore`: A `Semaphore` to control the capacity of the polling station.
    - `votingSemaphore`: A `Semaphore` to control access to the voting process.
  - **ExitPoll**:
    - `eligibleVoters`: A `List<Integer>` to store IDs of voters eligible for the exit poll.
  - **ElectionSimulationGUI**:
    - `tableModel`: A `DefaultTableModel` to store and display the election results.
  - **VoteCounter**:
    - `votesA`: An `int` to store the number of votes for candidate A.
    - `votesB`: An `int` to store the number of votes for candidate B.

- **Identify the operations that can be invoked on these structures, including:**
  - **PollingStation**:
    - `public synchronized void openStation()`
      - **Functionality**: Opens the polling station and notifies all waiting threads.
      - **Calling Entities**: `PollClerk`
    - `public synchronized boolean validateID(Voter v) throws InterruptedException`
      - **Functionality**: Validates the voter's ID and updates the GUI.
      - **Calling Entities**: `PollClerk`
    - `public void enterQueue(Voter v) throws InterruptedException`
      - **Functionality**: Adds a voter to the waiting queue.
      - **Calling Entities**: `Voter`
    - `public synchronized Voter dequeueVoter()`
      - **Functionality**: Removes and returns the next voter from the waiting queue.
      - **Calling Entities**: `PollClerk`
    - `public void castVote(Voter v) throws InterruptedException`
      - **Functionality**: Processes the voter's vote and updates the GUI.
      - **Calling Entities**: `Voter`
    - `public void processExit(Voter v) throws InterruptedException`
      - **Functionality**: Processes the voter's exit and updates the GUI.
      - **Calling Entities**: `Voter`
    - `public synchronized boolean hasVotersInside()`
      - **Functionality**: Checks if there are any voters inside the polling station.
      - **Calling Entities**: `PollClerk`
    - `public synchronized void closeStation()`
      - **Functionality**: Closes the polling station and notifies all waiting threads.
      - **Calling Entities**: `ElectionSimulationGUI`
    - `public synchronized boolean isOpen()`
      - **Functionality**: Checks if the polling station is open.
      - **Calling Entities**: `Voter`
    - `public synchronized boolean hasVoted(int voterID)`
      - **Functionality**: Checks if a voter has already voted.
      - **Calling Entities**: `Voter`

  - **ExitPoll**:
    - `public synchronized String conductExitPoll(int voterID) throws InterruptedException`
      - **Functionality**: Conducts an exit poll for a voter and returns the result.
      - **Calling Entities**: `Voter`, `Pollster`
    - `public boolean isActive()`
      - **Functionality**: Checks if the exit poll is active.
      - **Calling Entities**: `Pollster`
    - `public synchronized void close()`
      - **Functionality**: Closes the exit poll.
      - **Calling Entities**: `Main`
    - `public void registerVoter(int voterID)`
      - **Functionality**: Registers a voter for the exit poll.
      - **Calling Entities**: `Voter`
    - `public List<Integer> getEligibleVoters()`
      - **Functionality**: Returns the list of eligible voters.
      - **Calling Entities**: `Pollster`

  - **ElectionSimulationGUI**:
    - `public void updateTable(String door, String voter, String clerk, String booth, String scoreA, String scoreB, String exit)`
      - **Functionality**: Updates the table with the provided information.
      - **Calling Entities**: `PollingStation`, `Voter`
    - `public void updateBooth(String booth)`
      - **Functionality**: Updates the booth information in the table.
      - **Calling Entities**: `PollingStation`
    - `public void incrementScoreA()`
      - **Functionality**: Increments the score for candidate A and updates the table.
      - **Calling Entities**: `PollingStation`
    - `public void incrementScoreB()`
      - **Functionality**: Increments the score for candidate B and updates the table.
      - **Calling Entities**: `PollingStation`

  - **VoteCounter**:
    - `public synchronized void addVote(String candidate)`
      - **Functionality**: Adds a vote for the specified candidate.
      - **Calling Entities**: `PollingStation`
    - `public synchronized int getVotesA()`
      - **Functionality**: Returns the number of votes for candidate A.
      - **Calling Entities**: `Main`
    - `public synchronized int getVotesB()`
      - **Functionality**: Returns the number of votes for candidate B.
      - **Calling Entities**: `Main`
    - `public synchronized void resetVotes()`
      - **Functionality**: Resets the vote counts for both candidates.
      - 