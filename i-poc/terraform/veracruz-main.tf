provider "aws" {
  region = "eu-west-1"
}

data "aws_vpc" "vpc" {
  # This assumes that there is a default VPC
  default = true
}

resource "aws_security_group" "sg" {
  name   = "allow-${var.deployment_name}"
  vpc_id = data.aws_vpc.vpc.id

  ingress {
    description      = "allow_ssh"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
    from_port         = 22
    to_port           = 22
    protocol          = "tcp"
  }

  ingress {
    description      = "allow_inside"
    cidr_blocks      = ["172.16.0.0/12"]
    ipv6_cidr_blocks = ["::/0"]
    from_port         = 0
    to_port           = 0
    protocol          = "-1"
  }

  egress {
    from_port        = 0
    to_port          = 0
    protocol         = "-1"
    cidr_blocks      = ["0.0.0.0/0"]
    ipv6_cidr_blocks = ["::/0"]
  }

  tags = {
    Name = "allow-${var.deployment_name}"
  }
}

module "ssh_key_pair" {
  # tflint-ignore: terraform_module_pinned_source
  source                = "git::https://github.com/cloudposse/terraform-aws-key-pair.git?ref=master"
  namespace             = var.deployment_name
  stage                 = "prod"
  name                  = "k3s"
  ssh_public_key_path   = "ssh"
  generate_ssh_key      = "true"
  private_key_extension = ".pem"
  public_key_extension  = ".pub"
}

module "k3s" {
  source = "./k3s"

  providers = {
    aws = aws
  }

  assign_public_ip   = true
  deployment_name    = var.deployment_name
  instance_type      = var.AWS_EC2_instance_type
  agent_instance_type = var.AWS_EC2_agent_instance_type
  #x86_64 instance
  subnet_id          = var.AWS_VPC_subnet_id
  keypair_content    = module.ssh_key_pair.public_key
  security_group_ids = [aws_security_group.sg.id]
  kubeconfig_mode    = "644"
  letsencrypt_email  = var.letsencrypt_email

}

#resource "null_resource" "k3s-wait" {
#  provisioner "local-exec" {
#    command = "until [ ! -z \"$(wget https://${format("k3s.%s.sslip.io",substr(split(".",module.k3s.instance.public_dns)[0],4,-1))}/k3s-start.sh.${module.k3s.k3s_edge.result} -O - 2>/dev/null)\" ];do sleep 5;done"
#  }
#}

output "ssh_ec2_instance" {
  value = "${format("Access EC2 instance using ssh -i %s ubuntu@%s",module.ssh_key_pair.private_key_filename,module.k3s.instance.public_dns)}"
  description = "EC2 instance name allocated"
}

output "ssh_ec2_node_instance" {
  value = "${format("Access EC2 node instance using ssh -i %s ec2-user@%s",module.ssh_key_pair.private_key_filename,module.k3s.agent_instance.public_dns)}"
  description = "EC2 node instance name allocated"
}

variable "letsencrypt_email" {
  type        = string
  description = "email to be used in let's encrypt"
}

variable "AWS_EC2_instance_type" {
  type        = string
  description = "instance type to be used, default is t3.medium"
  default     = "t3.medium"
}

variable "AWS_EC2_agent_instance_type" {
  type        = string
  description = "instance type to be used for nodes, default is c5.xlarge"
  default     = "c5.xlarge"
}

variable "deployment_name" {
  type        = string
  description = "Prefix applied to all objects created by this terraform"
  default     = "veracruz-ipoc-testing"
}

variable "AWS_VPC_subnet_id" {
  type        = string
  description = "subnet_id use the default of the VPC if this is not defined"
  default     = ""
}

