package deti.sd.mt.ct.model;

public enum Direction {
    NORTH,
    EAST,
    SOUTH,
    WEST;

    //Returns the direction 180 degrees from this one.
    public Direction opposite() {
        return switch (this) {
            case NORTH -> SOUTH;
            case WEST  -> EAST;
            case SOUTH -> NORTH;
            case EAST  -> WEST;
        };
    }
}

