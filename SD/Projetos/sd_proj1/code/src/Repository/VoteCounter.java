package Repository;

public class VoteCounter {
    private int votesA = 0;
    private int votesB = 0;

    public synchronized void addVote(String candidate) {
        if (candidate.equals("A")) {
            votesA++;
        } else {
            votesB++;
        }
    }

    public synchronized int getVotesA() {
        return votesA;
    }

    public synchronized int getVotesB() {
        return votesB;
    }

    public synchronized void resetVotes() {
        votesA = 0;
        votesB = 0;
    }
}
