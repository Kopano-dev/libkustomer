#!/usr/bin/env groovy

pipeline {
	agent {
		dockerfile {
			filename 'Dockerfile.build'
		}
	}
	stages {
		stage('Bootstrap') {
			steps {
				echo 'Bootstrapping..'
				sh 'go version'
			}
		}
		stage('Prepare') {
			steps {
				echo 'Preparing..'
				sh './bootstrap.sh'
				sh './configure --prefix=/tmp'
			}
		}
		stage('Lint') {
			steps {
				echo 'Linting..'
				sh 'make lint-checkstyle'
			}
		}
		stage('Vendor') {
			steps {
				echo 'Fetching vendor dependencies..'
				sh 'make vendor'
			}
		}
		stage('Build') {
			steps {
				echo 'Building..'
				sh 'make DATE=reproducible'
				sh 'find ./.libs -type f -exec sha256sum {} \\;'
				sh 'make examples'
			}
		}
		stage('Test') {
			steps {
				echo 'Testing..'
				sh 'make test-xml-short'
			}
		}
		stage('Install') {
			steps {
				echo 'Installing..'
				sh 'make install'
			}
		}
		stage('Dist') {
			steps {
				echo 'Dist..'
				sh 'test -z "$(git diff --shortstat 2>/dev/null |tail -n1)" && echo "Clean check passed."'
				sh 'make check'
				sh 'make dist'
			}
		}
	}
	post {
		always {
			junit allowEmptyResults: false, testResults: 'test/tests.xml'

			recordIssues enabledForFailure: true, qualityGates: [[threshold: 100, type: 'TOTAL', unstable: true]], tools: [checkStyle(pattern: 'test/tests.lint.xml')]

			archiveArtifacts 'dist/*.tar.gz'
			cleanWs()
		}
	}
}
