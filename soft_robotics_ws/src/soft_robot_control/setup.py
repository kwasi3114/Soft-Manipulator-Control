from setuptools import find_packages, setup

package_name = 'soft_robot_control'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='kwasi',
    maintainer_email='kdebrahp@gmail.com',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            "sensor_reader = soft_robot_control.sensor_reader:main",
            "motor_control = soft_robot_control.motor_control:main"
        ],
    },
)
