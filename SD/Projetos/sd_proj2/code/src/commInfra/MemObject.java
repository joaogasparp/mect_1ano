package commInfra;

public abstract class MemObject<R> {

    protected int size;

    public MemObject(int size) {
        if (size <= 0) {
            throw new IllegalArgumentException("Invalid size");
        }
        this.size = size;
    }

    public int getSize() {
        return size;
    }

    public void setSize(int size) {
        if (size <= 0) {
            throw new IllegalArgumentException("Invalid size");
        }
        this.size = size;
    }

    public abstract void write(R item) throws MemException;

    public abstract R read() throws MemException;
}
