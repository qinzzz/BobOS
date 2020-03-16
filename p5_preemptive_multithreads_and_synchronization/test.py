from glob import glob
import os

tests = glob('*.cc')

def main():
    os.system('rm -f *.temp')
    selected_tests = options(tests)
    NUM_TESTS = int(input('How many times? '))
    print()
    for test in selected_tests:
        test_name = test[:-3]
        outfile = f'{test_name}.temp'
        print(f'Testing {test_name}...')
        for x in range(NUM_TESTS):
            os.system(f'make -s {test_name}.result; cat {test_name}.result >> {test_name}.temp')
        process_results(test_name, outfile, NUM_TESTS)

def process_results(test_name, results, NUM_TESTS):
    ''' count of the the test results '''
    counts = {}
    with open(results) as f:
        for line in f:
            result = line[:-1]
            if result not in counts:
                counts[result] = 0
            counts[result] += 1
        print(f'++++ {test_name} results: ++++')
        if 'pass' in counts:
            print(f"passed: {(counts['pass'] / NUM_TESTS) * 100}% ({counts['pass']}/{NUM_TESTS})")
        if 'fail' in counts:
            print(f"failed: {(counts['fail'] / NUM_TESTS) * 100}% ({counts['fail']}/{NUM_TESTS})")
        print()
        os.system(f'rm -f {test_name}.temp; make -s clean')

def options(test_files):
    ''' Prompt user for test selection '''
    print('Select Tests to Run:')
    print('[0] Run All (this could take a while)')
    for i, test in enumerate(test_files):
        print(f'[{i+1}] {test}')
    selected = int(input("Which test? "))
    if selected == 0:
        return test_files
    return [test_files[selected - 1]]
        

if __name__ == '__main__':
    main()