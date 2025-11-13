import setuptools

with open('../README.md', 'r') as f:
    long_description = f.read()

setuptools.setup(
    name='life-monitor-station',
    version='0.1.0',
    author='maxrt',
    description='Setup for life-monitor-station',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/maxrt101/life-monitor/tree/master',
    packages=['station'],
    entry_points={
        'console_scripts': [
            'life-monitor-station = station.main:main'
        ]
    },
    classifiers=[
        'Programming Language :: Python :: 3',
    ],
    python_requires='>=3.11',
)