import os

def count_lines(file_path):
    if not os.path.isfile(file_path):
        print(f"Erro: O arquivo '{file_path}' não foi encontrado.")
        return 0
    with open(file_path, 'r') as file:
        line_count = sum(1 for line in file)
    return line_count

if __name__ == "__main__":
    # Construir o caminho absoluto do arquivo
    base_dir = os.path.dirname(__file__)
    file_path = os.path.join(base_dir, 'samples-1000000000.txt')
    
    lines = count_lines(file_path)
    if lines > 0:
        print(f'O arquivo {file_path} tem {lines} linhas.')