package deti.sd.mt.ct.model;

public class Collision {

    /*
     * COLLISION LOGIC
     * --------------------------------------------------
     * NORTH | vs SOUTH | vs EAST | vs WEST |
     * --------------------------------------------------
     * STRAIGHT | Sl,Su | Es,El,Eu | all |
     * --------------------------------------------------
     * LEFT_TURN | all | Es,El,Eu | all |
     * --------------------------------------------------
     * RIGHT_TURN | Sl,Su | Es,Eu | Wu |
     * --------------------------------------------------
     * U_TURN | all | all | all |
     * --------------------------------------------------
     * 
     * --------------------------------------------------
     * SOUTH | vs NORTH | vs EAST | vs WEST |
     * --------------------------------------------------
     * STRAIGHT | Nl,Nu | all | Ws,Wl,Wu |
     * --------------------------------------------------
     * LEFT_TURN | all | all | Ws,Wl,Wu |
     * --------------------------------------------------
     * RIGHT_TURN | Nl,Nu | Eu | Ws,Wu |
     * --------------------------------------------------
     * U_TURN | all | all | all |
     * -------------------------------------------------
     * 
     * -------------------------------------------------
     * EAST | vs NORTH | vs SOUTH | vs WEST |
     * -------------------------------------------------
     * STRAIGHT | all | Ss,Sl,Su | Wl,Wu |
     * -------------------------------------------------
     * LEFT_TURN | all | Ss,Sl,Su | all |
     * -------------------------------------------------
     * RIGHT_TURN | Nu | Ss,Su | Wl,Wu |
     * -------------------------------------------------
     * U_TURN | all | all | all |
     * -------------------------------------------------
     * 
     * -------------------------------------------------
     * WEST | vs NORTH | vs SOUTH | vs EAST |
     * -------------------------------------------------
     * STRAIGHT | Ns,Nl,Nu | all | El,Eu |
     * -------------------------------------------------
     * LEFT_TURN | Ns,Nl,Nu | all | all |
     * -------------------------------------------------
     * RIGHT_TURN | Ns,Nu | Su | El,Eu |
     * -------------------------------------------------
     * U_TURN | all | all | all |
     * -------------------------------------------------
     */

    /**
     * Determines whether two vehicles collide inside the intersection.
     *
     * <p>
     * A collision occurs when two vehicles, entering from different directions
     * with specific move types, would occupy conflicting paths through the
     * intersection at the same time.
     * </p>
     *
     * @param entryDir the entry direction of the vehicle requesting to traverse
     * @param entryMT  the move type of the vehicle requesting to traverse
     * @param isecDir  the entry direction of the vehicle already inside the
     *                 intersection
     * @param isecMT   the move type of the vehicle already inside the intersection
     * @return {@code true} if the two vehicles would collide; {@code false}
     *         otherwise
     */
    public static boolean collides(Direction entryDir, MoveType entryMT, Direction isecDir, MoveType isecMT) {

        // Empty lane handling
        if (entryDir == null || entryMT == null || isecDir == null || isecMT == null) {
            return false;
        }

        // Any U_TURN MoveType
        if (entryMT == MoveType.U_TURN || isecMT == MoveType.U_TURN) {
            return true;
        }

        // ALL collisions
        if ((((entryDir == Direction.NORTH && isecDir == Direction.SOUTH)
                || (entryDir == Direction.SOUTH && isecDir == Direction.NORTH)
                || (entryDir == Direction.EAST && isecDir == Direction.WEST)
                || (entryDir == Direction.WEST && isecDir == Direction.EAST))
                && entryMT == MoveType.LEFT_TURN)
                || (((entryDir == Direction.NORTH && isecDir == Direction.WEST)
                        || (entryDir == Direction.SOUTH && isecDir == Direction.EAST)
                        || (entryDir == Direction.EAST && isecDir == Direction.NORTH)
                        || (entryDir == Direction.WEST && isecDir == Direction.SOUTH))
                        && (entryMT == MoveType.STRAIGHT || entryMT == MoveType.LEFT_TURN))) {
            return true;
        }
        // Xs,Xl collisions
        if (((entryDir == Direction.NORTH && isecDir == Direction.EAST)
                || (entryDir == Direction.SOUTH && isecDir == Direction.WEST)
                || (entryDir == Direction.EAST && isecDir == Direction.SOUTH)
                || (entryDir == Direction.WEST && isecDir == Direction.NORTH))
                && (entryMT == MoveType.STRAIGHT || entryMT == MoveType.LEFT_TURN)) {
            return (isecMT == MoveType.STRAIGHT || isecMT == MoveType.LEFT_TURN);
        }
        // Xl collisions
        if (((entryDir == Direction.NORTH && isecDir == Direction.SOUTH)
                || (entryDir == Direction.SOUTH && isecDir == Direction.NORTH)
                || (entryDir == Direction.EAST && isecDir == Direction.WEST)
                || (entryDir == Direction.WEST && isecDir == Direction.EAST))
                && (entryMT == MoveType.STRAIGHT || entryMT == MoveType.RIGHT_TURN)) {
            return isecMT == MoveType.LEFT_TURN;
        }
        // Xs collisions
        if (((entryDir == Direction.NORTH && isecDir == Direction.EAST)
                || (entryDir == Direction.SOUTH && isecDir == Direction.WEST)
                || (entryDir == Direction.EAST && isecDir == Direction.SOUTH)
                || (entryDir == Direction.WEST && isecDir == Direction.NORTH))
                && entryMT == MoveType.RIGHT_TURN) {
            return isecMT == MoveType.STRAIGHT;
        }

        return false;
    }

}