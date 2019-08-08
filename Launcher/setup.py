import setuptools

setuptools.setup(
        name="IbLauncher",
        version="@PROJECT_VERSION@",
        author="Vector Informatik",
        description="Tool for launching integrationbus simulations",
        packages=setuptools.find_packages(include=["iblauncher", "tests"]),
        package_data={'iblauncher': ['README.md', 'LICENSE']},
)
