package ex1_p3;

public class MemStack<T> extends MemObject<T> {
    private int topo;

    // construtor para inicializar a pilha com a capacidade dada
    public MemStack(int capacidade) {
        super(capacidade);
        topo = -1;
    }

    // escreve um objeto no topo da pilha
    @Override
    public void write(T obj) throws Exception {
        if (topo == capacidade - 1) {
            throw new Exception("Pilha cheia");
        }
        buffer[++topo] = obj;
    }

    // lê e remove um objeto do topo da pilha
    @Override
    public T read() throws Exception {
        if (topo == -1) {
            throw new Exception("Pilha vazia");
        }
        return buffer[topo--];
    }

    // verifica se a pilha está vazia
    public boolean estaVazia() {
        return topo == -1;
    }
}
