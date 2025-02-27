package ex1_p2;

public class StackObj<T> {
    private T[] pilha;
    private int topo;

    // construtor para inicializar a pilha com a capacidade dada
    @SuppressWarnings("unchecked")
    public StackObj(int capacidade) {
        pilha = (T[]) new Object[capacidade];
        topo = -1;
    }

    // adiciona um objeto ao topo da pilha
    public void empilhar(T obj) {
        if (topo < pilha.length - 1) {
            pilha[++topo] = obj;
        }
    }

    // remove e retorna o objeto do topo da pilha
    public T desempilhar() {
        if (topo >= 0) {
            return pilha[topo--];
        }
        return null; // retorna null se a pilha estiver vazia
    }

    // verifica se a pilha está vazia
    public boolean estaVazia() {
        return topo == -1;
    }
}
