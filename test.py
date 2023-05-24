from collections import defaultdict

data = []

with open('data/data-serialized.txt', 'r') as file:
    for line in file:
        values = line.strip().split(',')
        data.append(tuple(values))

exp3_data = defaultdict(lambda:(0, 0, 0, 0))
matrix_ucb_data = defaultdict(lambda:(0, 0, 0, 0))

def new(x, y):
    m = x[-1]
    n = y[-1]
    return tuple(
        (_ * m + __ * n) / (m + n) for _, __ in zip(x[:-1], y[:-1])
    ) + (m + n,)

"">
        << name << ','
        << seed << ','
        << depth_bound << ','
        << actions << ','
        << transitions << ','
        << chance_threshold << ','
        << iterations << ','
        << average_score << ','
        << matrix_node_count << ','
        << time_spent << ','
        << sample_size << ','
        << total_games << std::endl;
"">


# Print the data list
for entry in data:
    try:
        name, seed, depth_bound, actions, transitions, chance_threshold, iterations, average_score, matrix_node_count, time_spent, sample_size, total_games = entry
    except:
        continue
    time_spent = time_spent[:-2]
    key = (depth_bound, actions, transitions, chance_threshold, iterations)
    value = (average_score, matrix_node_count, time_spent, total_games)
    value = tuple(float(_) for _ in value)
    if name == 'exp3':
        old_value = exp3_data[key]
        new_value = new(value, old_value)
        exp3_data[key] = new_value
    if name == 'matrix_ucb':
        old_value = matrix_ucb_data[key]
        new_value = new(value, old_value)
        matrix_ucb_data[key] = new_value

total_expl = 0
count = 0
for key, value in matrix_ucb_data.items():
    value_ = exp3_data[key]

    (depth_bound, actions, transitions, chance_threshold, iterations) = key
    (average_score, matrix_node_count, time_spent, total_games) = value
    (average_score_, matrix_node_count_, time_spent_, total_games_) = value_


    if (int(depth_bound) > 0) : #lmao
        total_expl += value_[0]
        count += 1

        print('depth, actions, chance, thresh, iterations')
        print(key)
        print("expl: ", value_[0], value[0])
        print('count: ', int(value_[1]), int(value[1]))
        print('runtime (ms): ', int(value_[2]), int(value[2]))
        print('total games: ', int(value_[-1]), int(value[-1]))
        print()
print(total_expl / count)
