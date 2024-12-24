#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <time.h>
#include <random>
#include <unordered_set>
#include <algorithm>
using namespace std;


class MaxCliqueProblem
{
public:
    static int GetRandom(int a, int b)
    {
        static mt19937 generator;
        uniform_int_distribution<int> uniform(a, b);
        return uniform(generator);
    }

    void ReadGraphFile(string filename)
    {
        ifstream fin(filename);
        string line;
        int vertices = 0, edges = 0;
        while (getline(fin, line))
        {
            if (line[0] == 'c')
            {
                continue;
            }

            stringstream line_input(line);
            char command;
            if (line[0] == 'p')
            {
                string type;
                line_input >> command >> type >> vertices >> edges;
                neighbour_sets.resize(vertices);
            }
            else
            {
                int start, finish;
                line_input >> command >> start >> finish;
                // Edges in DIMACS file can be repeated, but it is not a problem for our sets
                neighbour_sets[start - 1].insert(finish - 1);
                neighbour_sets[finish - 1].insert(start - 1);
            }
        }
    }

    void FindClique(int randomization, int iterations)
{
    static mt19937 generator;

    int n = neighbour_sets.size(); // Количество вершин в графе

    unordered_set<int> current_clique; // Текущее множество вершин
    unordered_set<int> best_clique_set; // Лучшая найденная клика в виде множества

    for (int it = 0; it < iterations; ++it)
    {
        // Начинаем с произвольной вершины
        current_clique.clear();
        int start_vertex = GetRandom(0, n - 1);
        current_clique.insert(start_vertex);

        // Выполняем итерации локального поиска
        bool improved = true;
        while (improved)
        {
            improved = false;

            // Формируем список кандидатов на добавление в клику
            vector<int> candidates;
            for (int v = 0; v < n; ++v)
            {
                // Вершина не должна быть уже в клике, и она должна быть соседней ко всем вершинам текущей клики
                if (current_clique.count(v) == 0)
                {
                    bool is_candidate = true;
                    for (int u : current_clique)
                    {
                        if (neighbour_sets[u].count(v) == 0)
                        {
                            is_candidate = false;
                            break;
                        }
                    }
                    if (is_candidate)
                    {
                        candidates.push_back(v);
                    }
                }
            }

            // Если есть кандидаты, выбираем случайного
            if (!candidates.empty())
            {
                int idx = GetRandom(0, candidates.size() - 1);
                current_clique.insert(candidates[idx]);
                improved = true;
                continue;
            }

            // Если не можем ничего добавить, пробуем удалить "проблемную" вершину
            for (auto it = current_clique.begin(); it != current_clique.end(); ++it)
            {
                int vertex = *it;
                bool is_problematic = false;

                // Проверяем, есть ли несоответствие кликовых свойств
                for (int v : current_clique)
                {
                    if (v != vertex && neighbour_sets[vertex].count(v) == 0)
                    {
                        is_problematic = true;
                        break;
                    }
                }
                if (is_problematic)
                {
                    current_clique.erase(it);
                    improved = true;
                    break;
                }
            }
        }

        // Обновляем лучшую клику
        if (current_clique.size() > best_clique_set.size())
        {
            best_clique_set = current_clique;
        }
    }

    // Преобразуем лучшую клику из unordered_set в vector
    best_clique.assign(best_clique_set.begin(), best_clique_set.end());
}


    const vector<int>& GetClique()
    {
        return best_clique;
    }

    bool Check()
    {
        if (unique(best_clique.begin(), best_clique.end()) != best_clique.end())
        {
            cout << "Duplicated vertices in the clique\n";
            return false;
        }
        for (int i : best_clique)
        {
            for (int j : best_clique)
            {
                if (i != j && neighbour_sets[i].count(j) == 0)
                {
                    cout << "Returned subgraph is not a clique\n";
                    return false;
                }
            }
        }
        return true;
    }

private:
    vector<unordered_set<int>> neighbour_sets;
    vector<int> best_clique;
};

int main()
{
    int iterations;
    cout << "Number of iterations: ";
    cin >> iterations;
    int randomization;
    cout << "Randomization: ";
    cin >> randomization;
    // Список файлов для проверки
    vector<string> files = { "brock200_1.clq", "brock200_2.clq", "brock200_3.clq", "brock200_4.clq", "brock400_1.clq",
        "brock400_2.clq", "brock400_3.clq", "brock400_4.clq", "C125.9.clq", "gen200_p0.9_44.clq", "gen200_p0.9_55.clq", "hamming8-4.clq",
        "johnson16-2-4.clq", "johnson8-2-4.clq", "keller4.clq", "MANN_a27.clq", "MANN_a9.clq", "p_hat1000-1.clq",
        "p_hat1000-2.clq", "p_hat1000-2.clq", "p_hat1500-1.clq", "p_hat300-3.clq", "p_hat500-3.clq", "san1000.clq", "sanr200_0.9.clq",
        "sanr400_0.7.clq" };

    ofstream fout("clique.csv");
    fout << "File; Clique; Time (sec)\n";
    for (string file : files)
    {
        MaxCliqueProblem problem;
        problem.ReadGraphFile(file);
        clock_t start = clock();
        problem.FindClique(randomization, iterations);
        if (! problem.Check())
        {
            cout << "*** WARNING: incorrect clique ***\n";
            fout << "*** WARNING: incorrect clique ***\n";
        }
        fout << file << "; " << problem.GetClique().size() << "; " << double(clock() - start) / 1000 << '\n';
        cout << file << ", result - " << problem.GetClique().size() << ", time - " << double(clock() - start) / 1000 << '\n';
    }
    fout.close();
    return 0;
}
