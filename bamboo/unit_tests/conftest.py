import pytest, re, subprocess

def pytest_addoption(parser):
    cluster = re.sub('[0-9]+', '', subprocess.check_output('hostname'.split()).strip())
    default_dirname = subprocess.check_output('git rev-parse --show-toplevel'.split()).strip()
    default_exe = '%s/../LBANN-NIGHTD-BDE/build/%s.llnl.gov/model_zoo/lbann' % (default_dirname, cluster)
    parser.addoption('--exe', action='store', default=default_exe,
                     help='--exe specifies the executable')
    parser.addoption('--dirname', action='store', default=default_dirname,
                     help='--dirname specifies the top-level directory')

@pytest.fixture
def exe(request):
    return request.config.getoption('--exe')

@pytest.fixture
def dirname(request):
    return request.config.getoption('--dirname')
